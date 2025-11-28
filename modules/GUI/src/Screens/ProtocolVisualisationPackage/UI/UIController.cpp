#include "UIController.hpp"
#include "../../../Shared/UIFactory/UIFactory.hpp"
#include "../../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Libraries/magic_enum.hpp"
#include "../States/SimulationConfiguration.hpp"

UIController::UIController(tgui::Gui &gui) : gui(gui)
{
}

void UIController::setupUI(sf::View &networkView)
{
    tabContainer = UIFactory::createTabContainer({"100%", "100%"});
    tabContainer->setTabsHeight(50);
    tabContainer->setPosition({"0%", "0%"});

    clientPanel = tabContainer->addTab("Client");
    networkPanel = tabContainer->addTab("Network");
    logsPanel = tabContainer->addTab("Log");

    setClientPanelUI();
    setProtocolPanelUI(networkView);
    setLogsPanelUI();

    // must be called after setting the panels
    bindCallbacks();

    tabContainer->select(0);
    gui.add(tabContainer);
}

void UIController::updateUI(LiveNetworkState &state)
{
    //TODO:
    // communicationModeText->setText("Communication Mode: " + state.communicationMode);
    // timeText->setText("Time: " + std::to_string(state.tickNumber /* Config::TICK_DURATION*/) + "s");
}

void UIController::setServerStatus(bool connected)
{
    if (connected)
        {
            showSimulationReadyToStartUI();

        }
        else
        {
            hideAllSimulationControlButtons();


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

uint64_t UIController::getMaxSimulationTimeMs() const
{
    return unlimitedSimulationCheckbox->isChecked() ? 0 : std::stoull(maxSimulationTimeInput->getText().toStdString());
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
        // Check if unlimited simulation is enabled
        if (!unlimitedSimulationCheckbox->isChecked()) {
            // Validate max simulation time input only if not unlimited
            std::string timeText = maxSimulationTimeInput->getText().toStdString();

            try {
                // Try to parse as a number
                size_t pos;
                int timeValue = std::stoi(timeText, &pos);

                // Check if entire string was parsed and value is positive
                if (pos != timeText.length() || timeValue <= 0) {
                    auto errorBox = UIFactory::createMessageBox("Invalid Input",
                        "Max Simulation Time must be a positive number.");
                    errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                        if(button == "OK" ){
                            msgBox->getParent()->remove(msgBox->shared_from_this());
                        }
                    });
                    gui.add(errorBox);
                    return;
                }
            } catch (const std::exception&) {
                // Parsing failed
                auto errorBox = UIFactory::createMessageBox("Invalid Input",
                    "Max Simulation Time must be a valid positive number.");
                errorBox->onButtonPress([msgBox = errorBox.get()](const tgui::String &button){
                    if(button == "OK" ){
                        msgBox->getParent()->remove(msgBox->shared_from_this());
                    }
                });
                gui.add(errorBox);
                return;
            }
        }

        if (onStartSimulation) onStartSimulation();
        else {
            std::cerr << "No start simulation callback set!" << std::endl;
            return;
        }
            showSimulationStartedUI();

         });

    stopSimulationButton->onPress([this]()
                                  {
    if (onStopSimulation) onStopSimulation();
    else {
        std::cerr << "No stop simulation callback set!" << std::endl;
        return;
    }
    showSimulationStoppedUI();


                                  });

    restartSimulationButton->onPress([this]()
                                     {
    if (onRestartSimulation) onRestartSimulation();
    else {
        std::cerr << "No restart simulation callback set!" << std::endl;
        return;
    }
        showSimulationRestartedUI();
    });

    pauseBitmapButton->onPress([this]()
                                         {
    if (onPauseSimulation) onPauseSimulation();
    else {
std::cerr << "No pause simulation callback set!" << std::endl; 
return;
    } 
    showSimulationPausedUI();
                                         });

    resumeBitmapButton->onPress([this]()
                                          {
    if (onResumeSimulation) onResumeSimulation();
    else {
std::cerr << "No resume simulation callback set!" << std::endl;
return;
    } 
    showSimulationRunningUI();
                                         });

    scenarioTypeComboBox->onItemSelect([this](const tgui::String &item)
                                      {
        if (onScenarioTypeChanged) onScenarioTypeChanged(scenarioTypeComboBox->getSelectedItemIndex());
        else {
            std::cerr << "No scenario type changed callback set!" << std::endl;
            return;
        }
                                      });

    unlimitedSimulationCheckbox->onChange([this]()
                                          {
        bool isUnlimited = unlimitedSimulationCheckbox->isChecked();
        maxSimulationTimeLabel->setVisible(!isUnlimited);
        maxSimulationTimeInput->setVisible(!isUnlimited);
                                          });

    findButton->onPress([this]()
                       {
        if (onFindNode) {
            std::string query = searchBox->getText().toStdString();
            if (!query.empty()) {
                onFindNode(query);
            }
        }
        else {
            std::cerr << "No find node callback set!" << std::endl;
        }
                       });

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
            simulationCommandPanel->setVisible(true);
        });

    });
}

void UIController::setProtocolPanelUI(sf::View &networkView)
{
    auto simulationCommandTopPanel = UIFactory::createPanel({"35%", "8.5%"});
    simulationCommandTopPanel->setPosition({"0.75%", "0.75%"});
    networkPanel->add(simulationCommandTopPanel);

    auto simTopulationCommandLabel = UIFactory::createLabel("Simulation Controls:");
    simTopulationCommandLabel->setPosition({"2%", "2%"});
    simulationCommandTopPanel->add(simTopulationCommandLabel);

    pauseBitmapButton = UIFactory::createBitmapButton(ResourceManager::getInstance().getTguiTexture(TguiTextureKey::PauseButton), "", {"40", "40"});
    pauseBitmapButton->setPosition({"70%", "2%"});
    pauseBitmapButton->setImageScaling(1.0f);  
    pauseBitmapButton->setEnabled(false);
    pauseBitmapButton->setVisible(false);
    simulationCommandTopPanel->add(pauseBitmapButton);

    resumeBitmapButton = UIFactory::createBitmapButton(ResourceManager::getInstance().getTguiTexture(TguiTextureKey::PlayButton), "", {"40", "40"});
    resumeBitmapButton->setPosition({"70%", "2%"});
    resumeBitmapButton->setVisible(false);
    resumeBitmapButton->setImageScaling(1.0f);  
    resumeBitmapButton->setEnabled(false);
    simulationCommandTopPanel->add(resumeBitmapButton);



    simulationTimeProgressBar = UIFactory::createProgressBar({"18%", "6%"});
    simulationTimeProgressBar->setPosition({"77%", "2%"});
    simulationTimeProgressBar->setValue(0); // Initial value
    simulationTimeProgressBar->setText("Simulation Time");
    networkPanel->add(simulationTimeProgressBar);

    networkPanel->add(pauseBitmapButton);


    exportSimulationButton = UIFactory::createButton("Save");
    exportSimulationButton->setPosition({"80%", "2%"});
    exportSimulationButton->setSize({"7%", "4%"});
    exportSimulationButton->onPress([this]()
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
    networkPanel->add(exportSimulationButton);


    auto splitContainer = tgui::SplitContainer::create({"100%", "90%"});
    splitContainer->setPosition({"0%", "10%"});
    splitContainer->setOrientation(tgui::Orientation::Horizontal);
    splitContainer->setMinValidSplitterOffset("20%"); // Left widget can't be smaller than 20%
    splitContainer->setMaxValidSplitterOffset("50%"); // Right widget can't be smaller than 50% of the container
    splitContainer->setSplitterOffset("30%"); // Splitter is initially located at the center of the container
   networkPanel->add(splitContainer);
   
   nodeBrowserPanel = UIFactory::createPanel({"100%", "100%"});
   nodeBrowserPanel->setPosition({"0%", "0%"});
    splitContainer->add(nodeBrowserPanel);

    // Search/Find functionality
    searchBox = tgui::EditBox::create();
    searchBox->setPosition({"40%", "2%"});
    searchBox->setSize({"15%", "4%"});
    searchBox->setDefaultText("Node ID or x,y");
    nodeBrowserPanel->add(searchBox);

    findButton = UIFactory::createButton("Find");
    findButton->setPosition({"56%", "2%"});
    findButton->setSize({"7%", "4%"});
    nodeBrowserPanel->add(findButton);


    
    canvas = tgui::CanvasSFML::create();
    canvas->setPosition({"0%", "10%"});
    canvas->setSize({"100%", "90%"});
    canvas->setView(networkView);
    canvas->clear(tgui::Color(30, 30, 30));
    splitContainer->add(canvas);
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



void UIController::hideAllSimulationControlButtons()
{
            serverStatusConnected->setVisible(false);
            confFileSelectionGroup->setVisible(false);
            serverStatusDisconnected->setVisible(true);
            startSimulationButton->setEnabled(false);
            stopSimulationButton->setVisible(false);
            restartSimulationButton->setVisible(false);

            tgui::String fileNameLabelString= "No File Selected";
            fileNameLabel->setText(fileNameLabelString);
            pauseBitmapButton->setVisible(false);
            resumeBitmapButton->setVisible(false);
}

void UIController::showSimulationStartedUI()
{
    startSimulationButton->setVisible(false);
    confFileSelectionButton->setEnabled(false);
    pauseBitmapButton->setEnabled(true);
    pauseBitmapButton->setVisible(true);
    stopSimulationButton->setVisible(true);
    restartSimulationButton->setVisible(true);
    scenarioTypeComboBox->setEnabled(false);
    unlimitedSimulationCheckbox->setEnabled(false);
    maxSimulationTimeInput->setEnabled(false);
}

void UIController::showSimulationRunningUI()
{    pauseBitmapButton->setEnabled(true);
    resumeBitmapButton->setEnabled(false);
    pauseBitmapButton->setVisible(true);
    resumeBitmapButton->setVisible(false);
}

void UIController::showSimulationPausedUI()
{
        pauseBitmapButton->setEnabled(false);
    resumeBitmapButton->setEnabled(true);
    pauseBitmapButton->setVisible(false);
    resumeBitmapButton->setVisible(true);
}

void UIController::showSimulationStoppedUI()
{
    stopSimulationButton->setVisible(false);
    restartSimulationButton->setVisible(false);
    scenarioTypeComboBox->setEnabled(true);
    startSimulationButton->setVisible(true);
    startSimulationButton->setEnabled(false);
    pauseBitmapButton->setVisible(false);
    resumeBitmapButton->setVisible(false);
    confFileSelectionButton->setEnabled(true);
    unlimitedSimulationCheckbox->setEnabled(true);
    maxSimulationTimeInput->setEnabled(true);
    setFileNameLabel("No File Selected");
}

void UIController::showSimulationRestartedUI()
{
    startSimulationButton->setVisible(false);
    pauseBitmapButton->setVisible(true);
    resumeBitmapButton->setVisible(false);
    stopSimulationButton->setVisible(true);
}

void UIController::showSimulationReadyToStartUI()
{
    serverStatusConnected->setVisible(true);
    confFileSelectionGroup->setVisible(true);
    confFileSelectionButton->setEnabled(true);      
    serverStatusDisconnected->setVisible(false);
    stopSimulationButton->setVisible(false);
    startSimulationButton->setVisible(true);
    startSimulationButton->setEnabled(true);
    restartSimulationButton->setVisible(false);
}


void UIController::setClientPanelUI()
{
     backButton = UIFactory::createButton("Back");
    clientPanel->add(backButton);
    backButton->setPosition({"2%", "3%"});
    backButton->setSize({"8%", "4%"});

    serverStatusConnected = UIFactory::createLabel("Simulator Connected!");
    serverStatusConnected->setPosition({"12%", "3%"});
    serverStatusConnected->setVisible(false);
    clientPanel->add(serverStatusConnected);

    serverStatusDisconnected = UIFactory::createLabel("Simulator disconnected...");
    serverStatusDisconnected->setPosition({"12%", "3%"});
    clientPanel->add(serverStatusDisconnected);


    confFileSelectionGroup = UIFactory::createPanel({"70%", "45%"});
    confFileSelectionGroup->setPosition({"2%", "10%"});
    clientPanel->add(confFileSelectionGroup);
    confFileSelectionGroup->setVisible(false);

    auto configurationLabel = UIFactory::createLabel("Configuration Selection");;
    configurationLabel->setPosition({"2%", "2%"});
    configurationLabel->setSize({"30%", "12%"});
    confFileSelectionGroup->add(configurationLabel);

    auto confFileLabel = UIFactory::createLabel("Configuration File:");
    confFileSelectionGroup->add(confFileLabel);
    confFileLabel->setPosition({"2%", "23%"});
    confFileLabel->setSize({"20%", "12%"});


    confFileSelectionButton = UIFactory::createButton("Select File");
    confFileSelectionGroup->add(confFileSelectionButton);
    confFileSelectionButton->setPosition({"25%", "33%"});
    confFileSelectionButton->setSize({"20%", "12%"});

    fileNameLabel = UIFactory::createLabel("No configuration file selected");
    fileNameLabel->setPosition({"25%", "18%"});
    fileNameLabel->setSize({"70%", "12%"});
    confFileSelectionGroup->add(fileNameLabel);

    scenarioTypeComboBox = UIFactory::createEnumComboBox({"20%", "12%"});
    confFileSelectionGroup->add(scenarioTypeComboBox);
    scenarioTypeComboBox->setPosition({"2%", "50%"});
        // Add each enum value by name
    for (auto mode : magic_enum::enum_values<ScenarioType>())
    {
        std::string label = std::string(magic_enum::enum_name(mode));
        scenarioTypeComboBox->addItem(label);
    }
    scenarioTypeComboBox->setSelectedItemByIndex(0); // Select the first item by default

    unlimitedSimulationCheckbox = UIFactory::createCheckBox("Unlimited Simulation Time", false);
    confFileSelectionGroup->add(unlimitedSimulationCheckbox);
    unlimitedSimulationCheckbox->setPosition({"2%", "70%"});
    unlimitedSimulationCheckbox->setSize({"3%", "8%"});

    maxSimulationTimeLabel = UIFactory::createLabel(" / Max Simulation Time (ms):");
    confFileSelectionGroup->add(maxSimulationTimeLabel);
    maxSimulationTimeLabel->setPosition({"50%", "70%"});
    maxSimulationTimeLabel->setSize({"30%", "12%"});

    maxSimulationTimeInput = UIFactory::createTypeableInput("3600000", {"20%", "12%"});
    confFileSelectionGroup->add(maxSimulationTimeInput);
    maxSimulationTimeInput->setPosition({"75%", "70%"});
    maxSimulationTimeInput->setDefaultText("3600000");
    maxSimulationTimeInput->setText("3600000");



    simulationCommandPanel = UIFactory::createPanel({"70%", "40%"});
    simulationCommandPanel->setPosition({"2%", "50%"});
    clientPanel->add(simulationCommandPanel);
    simulationCommandPanel->setVisible(false);


    auto commandLabel = UIFactory::createLabel("Simulation Commands");
    commandLabel->setPosition({"2%", "2%"});
    commandLabel->setSize({"30%", "12%"});
    simulationCommandPanel->add(commandLabel);

    startSimulationButton = UIFactory::createButton("Start Simulation");
    simulationCommandPanel->add(startSimulationButton);
    startSimulationButton->setPosition({"2%", "20%"});
    startSimulationButton->setSize({"20%", "12%"});
    startSimulationButton->setEnabled(false); // Initially disabled until a file is selected

    stopSimulationButton = UIFactory::createButton("Stop Simulation");
    simulationCommandPanel->add(stopSimulationButton);
    stopSimulationButton->setPosition({"2%", "35%"});
    stopSimulationButton->setSize({"20%", "12%"});

    restartSimulationButton = UIFactory::createButton("Restart Simulation");
    simulationCommandPanel->add(restartSimulationButton);
    restartSimulationButton->setPosition({"2%", "50%"});
    restartSimulationButton->setSize({"20%", "12%"});


  }
