#include "UIController.hpp"
#include "../../../UI/UIFactory/UIFactory.hpp"

UIController::UIController(tgui::Gui &gui) : gui(gui)
{
}

void UIController::setupUI(sf::View &networkView)
{
    tabContainer = UIFactory::createTabContainer({"92%", "92%"});
    tabContainer->setTabsHeight(50);
    tabContainer->setPosition({"4%", "6%"});

    serverPanel = tabContainer->addTab("Server");
    networkPanel = tabContainer->addTab("Network");
    logsPanel = tabContainer->addTab("Log");
    metricsPanel = tabContainer->addTab("Metrics");

    setServerPanelUI();
    setNetworkPanelUI(networkView);
    setLogsPanelUI();
    setMetricsPanelUI();

    // must be called after setting the panels
    bindCallbacks();

    tabContainer->select(0);
    gui.add(tabContainer);
}

void UIController::updateUI(ProtocolVisualisationState &state)
{
     communicationModeText->setText("Communication Mode: " + state.communicationMode);
    timeText->setText("Time: " + std::to_string(state.tickNumber /* Config::TICK_DURATION*/) + "s");
}

void UIController::setServerStatus(bool connected)
{
    if (connected)
        {
            serverStatusConnected->setVisible(true);
            confFileSelectionGroup->setVisible(true);
            serverStatusDisconnected->setVisible(false);
        }
        else
        {
            serverStatusConnected->setVisible(false);
            confFileSelectionGroup->setVisible(false);
            serverStatusDisconnected->setVisible(true);
            startSimulationButton->setEnabled(false);
            tgui::String fileNameLabelString= "No File Selected";
            fileNameLabel->setText(fileNameLabelString);
        }
}

void UIController::setFileNameLabel(const std::string &name)
{
    fileNameLabel->setText(name);
}

void UIController::enableStartButton(bool enabled)
{
    startSimulationButton->setEnabled(enabled);
}

void UIController::enablePauseResumeButton(bool enabled)
{
    pauseResumeSimulationButton->setEnabled(enabled);
}

void UIController::setPauseResumeText(const std::string &text)
{
    pauseResumeSimulationButton->setText(text);
}

void UIController::errorMessageBox(const std::string &message)
{
    auto errorBox = UIFactory::createMessageBox("Error", message);
    errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
        if(button == "OK" ){
            msgBox->getParent()->remove(msgBox->shared_from_this()); 
            }
        });
    gui.add(errorBox);
}

void UIController::bindCallbacks()
{

    startSimulationButton->onPress([this]()
                                   {
        if (onStartSimulation) onStartSimulation();
        else std::cerr << "No start simulation callback set!" << std::endl;

        startSimulationButton->setEnabled(false);
        pauseResumeSimulationButton->setText("Pause Simulation");
        pauseResumeSimulationButton->setEnabled(true); });

    pauseResumeSimulationButton->onPress([this]()
                                         {
    if (onPauseResumeSimulation) onPauseResumeSimulation();
    else std::cerr << "No pause/resume simulation callback set!" << std::endl; });


    //Open the file Dialog
    confFileSelectionButton->onPress([this]()
                                     {
        auto targetDir = std::filesystem::path("output/topologies");
        auto openFileDialog = tgui::FileDialog::create("Open file", "Open");
        openFileDialog->setMultiSelect(false);
        openFileDialog->setPath(targetDir.string()); // Set default dir
        openFileDialog->setFileTypeFilters({{"Simulation File", {"*.simcfg"}}}, 1);

        openFileDialog->setTitle("Open Topology Configuration");
        openFileDialog->setSize({"50%", "50%"});
        openFileDialog->setPosition({"25%", "25%"});
        gui.add(openFileDialog);
        unsigned int textSize = 0.015f * gui.getView().getRect().height; // e.g. 2.5% of height
        openFileDialog->setTextSize(textSize);
        openFileDialog->getRenderer()->setTextSize(textSize);


        //When User select a File
        openFileDialog->onFileSelect([this](const std::vector<tgui::Filesystem::Path>& paths)
        {
            tgui::Filesystem::Path selectedPath = paths[0];
            std::string fileName = selectedPath.getFilename().toStdString();
            if (onFileSelected) onFileSelected(paths[0].asString().toStdString());
            else std::cerr << "No file selected callback set!" << std::endl;
        });

    });
}

void UIController::setNetworkPanelUI(sf::View &networkView)
{
    canvas = tgui::CanvasSFML::create();
    canvas->setPosition({"0%", "10%"});
    canvas->setSize({"100%", "90%"});
    canvas->setView(networkView);
    canvas->clear(tgui::Color(30, 30, 30));
    networkPanel->add(canvas);

    timeText = UIFactory::createLabel("Time: 0s");
    timeText->setPosition({"2%", "2%"});
    networkPanel->add(timeText);

    communicationModeText = UIFactory::createLabel("Communication Mode: ");
    communicationModeText->setPosition({"10%", "2%"});
    networkPanel->add(communicationModeText);

    auto buttonRouting = UIFactory::createButton("Routing");
    buttonRouting->setPosition({"90%", "2%"});
    buttonRouting->setSize({"7%", "4%"});
    buttonRouting->onPress([this]()
                           { routingDisplayEnabled = !routingDisplayEnabled; });
    networkPanel->add(buttonRouting);

    auto buttonSave = UIFactory::createButton("Save");
    buttonSave->setPosition({"80%", "2%"});
    buttonSave->setSize({"7%", "4%"});
    buttonSave->onPress([this]()
                        {
                            // MAKE A LAMBDA TO EXPORT THE METRICS
                            // EXACT IMPLEMENTATION NOT HERE
                            // TODO

                            // try
                            // {
                            //     CsvMetricWriter writer;
                            //     writer.writeNetworkMetricsToCsv("output/network_state.csv", *this, state);
                            //     std::cout << "Network state saved to network_state.csv\n";
                            // }
                            // catch (const std::exception &e)
                            // {
                            //     std::cerr << "Error writing CSV: " << e.what() << '\n';
                            // };
                        });
    networkPanel->add(buttonSave);
}

void UIController::setLogsPanelUI()
{

        // todo: display the metrics in the UI
    //     energyExpenditure->setString(energyExpenditureString + std::to_string(state.energyExp));
    //     window.draw(*energyExpenditure);

    //     nbRetransmission->setString(nbRetransmissionString + std::to_string(state.retransmissions));
    //     window.draw(*nbRetransmission);

    //     pdrText->setString(pdrString +  std::to_string(state.totalDataPacketsSent > 0 ? static_cast<float>(state.totalDataPacketsSent -state.retransmissions) / state.totalDataPacketsSent * 100 : 0) + "%");
    //     window.draw(*pdrText);

    //     // Draw logs and also get rid of the oldest log messages if the limit is reached
    //     {
    //         std::lock_guard<std::mutex> lock(state.logMutex);

    //         float y = 940.0f;

    //         if (state.logMessages.size() > 10)
    //         {
    //             state.logMessages.erase(state.logMessages.begin(), state.logMessages.end() - 10);
    //         }

    //         for (auto it = state.logMessages.rbegin(); it != state.logMessages.rend(); ++it)
    //         {
    //             sf::Text text(*font, *it, 10);
    //             text.setFillColor(sf::Color::White);
    //             text.setPosition(sf::Vector2f(10.0f, y));
    //             window.draw(text);
    //             y -= 15.0f;
    //         }
    //     }


}

void UIController::setMetricsPanelUI()
{
}

void UIController::setServerPanelUI()
{

    auto serverLabel = UIFactory::createLabel("Server");
    serverLabel->setPosition({"2%", "2%"});
    serverLabel->setSize({"20%", "6%"});
    serverPanel->add(serverLabel);

    serverStatusConnected = UIFactory::createLabel("Simulator Connected");
    serverStatusConnected->setPosition({"2%", "10%"});
    serverPanel->add(serverStatusConnected);

    serverStatusDisconnected = UIFactory::createLabel("Simulator disconnected...");
    serverStatusDisconnected->setPosition({"2%", "10%"});
    serverPanel->add(serverStatusDisconnected);
    serverStatusDisconnected->setVisible(false);

    confFileSelectionGroup = tgui::Group::create();
    serverPanel->add(confFileSelectionGroup);

    auto confFileLabel = UIFactory::createLabel("Configuration File:");
    confFileSelectionGroup->add(confFileLabel);
    confFileLabel->setPosition({"2%", "20%"});
    confFileLabel->setSize({"20%", "6%"});

    confFileSelectionButton = UIFactory::createButton("Select File");
    confFileSelectionGroup->add(confFileSelectionButton);
    confFileSelectionButton->setPosition({"25%", "20%"});
    confFileSelectionButton->setSize({"20%", "6%"});

    fileNameLabel = UIFactory::createLabel("No configuration file selected");
    fileNameLabel->setPosition({"25%", "15%"});
    fileNameLabel->setSize({"50%", "6%"});
    confFileSelectionGroup->add(fileNameLabel);

    startSimulationButton = UIFactory::createButton("Start Simulation");
    confFileSelectionGroup->add(startSimulationButton);
    startSimulationButton->setPosition({"2%", "40%"});
    startSimulationButton->setSize({"20%", "6%"});
    startSimulationButton->setEnabled(false); // Initially disabled until a file is selected

    pauseResumeSimulationButton = UIFactory::createButton("Pause Simulation");
    confFileSelectionGroup->add(pauseResumeSimulationButton);
    pauseResumeSimulationButton->setPosition({"2%", "65%"});
    pauseResumeSimulationButton->setSize({"20%", "6%"});
    pauseResumeSimulationButton->setEnabled(false); // Initially disabled until a simulation is running
}
