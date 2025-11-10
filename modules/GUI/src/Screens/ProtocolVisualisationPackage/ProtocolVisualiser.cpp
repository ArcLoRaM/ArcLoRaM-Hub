#include "ProtocolVisualiser.hpp"
#include "../../Shared/TopologyConfigIO/TopologyConfigIO.hpp"

ProtocolVisualiser::ProtocolVisualiser(tgui::Gui &gui)
: gui(gui), uiController(gui)
{
    routingManager.setDeviceManager(&deviceManager);

}

void ProtocolVisualiser::setup(sf::View &view)
{
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
            if (TopologyConfigIO::readToTopologyFileState(path,topologyState)) {
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
