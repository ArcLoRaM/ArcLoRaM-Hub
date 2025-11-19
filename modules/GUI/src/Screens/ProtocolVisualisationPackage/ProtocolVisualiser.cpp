#include "ProtocolVisualiser.hpp"
#include "../../Shared/TopologyConfigIO/TopologyConfigIO.hpp"

ProtocolVisualiser::ProtocolVisualiser(tgui::Gui &gui)
: gui(gui), uiController(gui)
{
    routingManager.setDeviceManager(&deviceManager);

}

void ProtocolVisualiser::setup(sf::View &view)
{
    networkView = &view;
    uiController.setupUI(view);
    bindUIEvents();
    clientConnectionMonitor.onStatusChanged = [this](bool connected) {
        //todo: change the name?
        uiController.setServerStatus(connected);

        //entering New Connection, clean start
        if(connected){
            deviceManager.clear();
            routingManager.clear();
            animationManager.clear();
            protocolState.resetState();
            topologyState.resetState();
        }
    };
    clientConnectionMonitor.start();
}

void ProtocolVisualiser::update(InputManager &inputManager)
{
    animationManager.update();
    deviceManager.update(inputManager, gui, uiController.getCanvas());
    uiController.updateUI(protocolState);
}

void ProtocolVisualiser::draw(sf::RenderWindow &window, sf::View &view)
{
    auto canvas = uiController.getCanvas();
    canvas->setView(view);
    canvas->clear(tgui::Color(30, 30, 30));


    // Draw scene elements
    deviceManager.draw(canvas);
    if(uiController.getRoutingDisplayEnabled()) routingManager.drawRootings(canvas);

    animationManager.draw(canvas);


    canvas->display();
}

void ProtocolVisualiser::bindUIEvents()
{
    uiController.onStartSimulation = [this]() {
        simulationController.startSimulation(topologyState);
    };

    uiController.onPauseSimulation = [this]() {
        if(simulationController.isRunning())
            simulationController.pauseSimulation();
    };
    
    uiController.onResumeSimulation = [this]() {
        if (!simulationController.isRunning()) 
        simulationController.resumeSimulation();
    };

    uiController.onScenarioTypeChanged = [this](const int scenarioId) {
        topologyState.setScenarioType(static_cast<ScenarioType>(scenarioId));
    };
    uiController.onRestartSimulation = [this]() {
        simulationController.restartSimulation();

        // Clear existing protocol visual elements
        animationManager.clear();
        deviceManager.clear();
        routingManager.clear();

        //Clear the simulation state
        protocolState.resetState();

    };

    uiController.onStopSimulation = [this]() {
        if(simulationController.isRunning())
        
            simulationController.stopSimulation();
                // Clear existing protocol visual elements
        animationManager.clear();
        deviceManager.clear();
        routingManager.clear();

        //Clear simulation state and configuration
        protocolState.resetState();
        topologyState.resetState();
    };

    uiController.onFindNode = [this](const std::string& searchQuery) {
        findNode(searchQuery);
    };

    uiController.onFileSelected = [this](const std::string& path) {

        //If a simulation is already running, make sure user wants to stop before launching a new one TODO
        //Make the "Are you sure Widget?" Reusable accross different context
        if(simulationController.isRunning()){
            /*
                    //TODO: stop the simulation before launching a new one !
        //right now it pauses it ....
        //     auto warningBox = UIFactory::createMessageBox("Warning", "A simulation is currently running. Are you sure you want to select a new configuration file?");
        //         warningBox->onButtonPress([msgBox = warningBox.get()](const tgui::String &button){
        //             if(button == "NO" ){
        //                     return;
        //             } 
        //         });
        //         gui.add(warningBox);
        //     pauseResumeSimulationButton->setEnabled(false);
        //     pauseResumeSimulationButton->setText("Pause Simulation");

        // std::cout << "Pausing simulation..." << std::endl;
        // sf::Packet basePacket;
        // pauseCommandPacket pausePacket;
        // basePacket << pausePacket;
        // TcpServer::instance().transmitPacket(basePacket); 
        // isSimulationRunning.store(false);

        */
        }

        if(!path.empty()){
            if (TopologyConfigIO::readTopologyConfig(path,topologyState)) {
                uiController.enableStartButton(true);
                uiController.setFileNameLabel("File Selected: " + path);
            }
            else{
                uiController.errorMessageBox("Failed to load topology configuration");                                             
                uiController.enableStartButton(false);
                uiController.setFileNameLabel("Invalid File: " + path);

            }
        }
    };
}

void ProtocolVisualiser::findNode(const std::string& searchQuery)
{
    if (!networkView) {
        std::cerr << "Network view not initialized!" << std::endl;
        return;
    }

    // Check for comma first - if present, parse as coordinates (x,y)
    size_t commaPos = searchQuery.find(',');
    if (commaPos != std::string::npos) {
        try {
            std::string xStr = searchQuery.substr(0, commaPos);
            std::string yStr = searchQuery.substr(commaPos + 1);

            // Trim whitespace
            xStr.erase(0, xStr.find_first_not_of(" \t"));
            xStr.erase(xStr.find_last_not_of(" \t") + 1);
            yStr.erase(0, yStr.find_first_not_of(" \t"));
            yStr.erase(yStr.find_last_not_of(" \t") + 1);

            float x = std::stof(xStr);
            float y = std::stof(yStr);

            networkView->setCenter({ x, y });
            uiController.getCanvas()->setView(*networkView);
            std::cout << "Moved view to coordinates (" << x << ", " << y << ")" << std::endl;
        } catch (const std::exception& e) {
            uiController.errorMessageBox("Invalid coordinates format! Use: x,y (e.g., 100,200)");
        }
        return;
    }

    // No comma found - try to parse as node ID (integer)
    try {
        int nodeId = std::stoi(searchQuery);

        // Get device position from DeviceManager
        auto positionOpt = deviceManager.getDevicePosition(nodeId);
        if (positionOpt.has_value()) {
            networkView->setCenter(positionOpt.value());
            uiController.getCanvas()->setView(*networkView);
            std::cout << "Found node " << nodeId << " at position ("
                     << positionOpt.value().x << ", " << positionOpt.value().y << ")" << std::endl;
        } else {
            uiController.errorMessageBox("Node " + std::to_string(nodeId) + " not found!");
        }
    } catch (const std::invalid_argument&) {
        uiController.errorMessageBox("Invalid input! Enter a Node ID or coordinates (x,y)");
    } catch (const std::out_of_range&) {
        uiController.errorMessageBox("Node ID out of range!");
    }
}
