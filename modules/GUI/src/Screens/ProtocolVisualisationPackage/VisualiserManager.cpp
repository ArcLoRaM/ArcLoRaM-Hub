

#include "VisualiserManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"
#include "CsvMetricWriter.hpp"
#include "../../UI/UIFactory/UIFactory.hpp"
#include "../../Network/TcpServer/ClientSession.hpp"


VisualiserManager::VisualiserManager(ProtocolVisualisationState &state, tgui::Gui &gui) : state(state), gui(gui), commandSender()

{

}

VisualiserManager::~VisualiserManager()
{
    isRoutineServerRunning.store(false);
    if (routineServer.joinable())
    {
        routineServer.join();
    }
}
void VisualiserManager::routineServerLoop() {
    while (isRoutineServerRunning.load()) {
        auto& s = ClientSession::instance();


        if(s.isConnected()) {
            serverStatusConnected->setVisible(true);
            serverStatusDisconnected->setVisible(false);
        }
        else {
            serverStatusConnected->setVisible(false);
            serverStatusDisconnected->setVisible(true); 
        }
        // std::cout << "Server connection status updated with:" << (s.isConnected() ? "Connected" : "Disconnected") << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
void VisualiserManager::setupUI(sf::View &networkView){

    tabContainer = UIFactory::createTabContainer({"92%", "92%"});
    tabContainer->setTabsHeight(50);
    tabContainer->setPosition({"4%", "6%"});

    serverPanel=tabContainer->addTab("Server");
    networkPanel=tabContainer->addTab("Network");
    logsPanel=tabContainer->addTab("Log");
    metricsPanel=tabContainer->addTab("Metrics");

    setServerPanelUI();
     setNetworkPanelUI(networkView);
    // setLogsPanelUI();
    // setMetricsPanelUI();

    tabContainer->select(0);
    gui.add(tabContainer);
}

void VisualiserManager::setNetworkPanelUI(sf::View &networkView)
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
                            try
                            {
                                CsvMetricWriter writer;
                                writer.writeNetworkMetricsToCsv("output/network_state.csv", *this, state);
                                std::cout << "Network state saved to network_state.csv\n";
                            }
                            catch (const std::exception &e)
                            {
                                std::cerr << "Error writing CSV: " << e.what() << '\n';
                            };
                        });
    networkPanel->add(buttonSave);
}

void VisualiserManager::setLogsPanelUI()
{
}

void VisualiserManager::setMetricsPanelUI()
{
}

void VisualiserManager::setServerPanelUI()
{
    auto serverLabel = UIFactory::createLabel("Server");
    serverLabel->setPosition({"2%", "2%"});
    serverLabel->setSize({"20%", "6%"});
    serverPanel->add(serverLabel);


    serverStatusConnected=UIFactory::createLabel("Simulator Connected");
    serverStatusConnected->setPosition({"2%", "10%"});
    serverPanel->add(serverStatusConnected);

    serverStatusDisconnected=UIFactory::createLabel("Simulator disconnected...");
    serverStatusDisconnected->setPosition({"2%", "10%"});
    serverPanel->add(serverStatusDisconnected);
    serverStatusDisconnected->setVisible(false);
    
    //launch the routine that will check client-server connection
    isRoutineServerRunning.store(true);
    routineServer = std::thread(&VisualiserManager::routineServerLoop, this);
}

void VisualiserManager::addDevice(std::unique_ptr<Device> device)
{
    std::lock_guard<std::mutex> lock(devicesMutex);

    if(!device)
    {
        throw std::runtime_error("Error: Attempted to add a null device.");
    }
        int nodeId = device->getNodeId();

    if (devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << device->getNodeId() << " already exists." << std::endl;
        return; // Device already exists, do not add
    }

    devices[nodeId] = std::move(device);


}

void VisualiserManager::addArrow(std::unique_ptr<Arrow> arrow)
{
    std::lock_guard<std::mutex> lock(arrowsMutex);
    arrows.push_back(std::move(arrow));
}

void VisualiserManager::addReceptionIcon(std::unique_ptr<ReceptionIcon> receptionIcon)
{
    std::lock_guard<std::mutex> lock(receptionIconsMutex);
    receptionIcons.push_back(std::move(receptionIcon));
}

void VisualiserManager::addDropAnimation(std::unique_ptr<PacketDrop> dropAnimation)
{
    std::lock_guard<std::mutex> lock(dropAnimationsMutex);
    dropAnimations.push_back(std::move(dropAnimation));
}

void VisualiserManager::changeArrowState(int senderId, int receiverId, std::string state)
{
    std::lock_guard<std::mutex> lock(arrowsMutex);
    if (state != "interference" && state != "notListening" && state != "received")
    {
        throw std::runtime_error("Error: state not recognized");
    }
    for (auto &arrow : arrows)
    {
        if (arrow->SenderId == senderId && arrow->ReceiverId == receiverId)
        {
            arrow->receptionState = state;
        }
    }
}

void VisualiserManager::startBroadcast(const sf::Vector2f &startPosition, float duration)
{
    std::lock_guard<std::mutex> lock(broadcastAnimationsMutex);
    broadcastAnimations.push_back(std::make_unique<BroadcastAnimation>(startPosition, duration));
}



void VisualiserManager::addRouting(int id1, int id2)
{
    // ID1 ---> ID2 , path towards ID2
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (devices.contains(id1) && devices.contains(id2))
    {
        routings[id1].insert(id2); // Only store id2 in id1's adjacency list
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
    }

}

void VisualiserManager::removeRouting(int id1, int id2)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (devices.contains(id1) && devices.contains(id2))
    {
        routings[id1].erase(id2);
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
    }
}

void VisualiserManager::addRetransmission(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to add retransmission." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }
    devices[nodeId]->metrics.incrementRetransmission();


}

void VisualiserManager::incrementPacketSent(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to add PacketSent." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }

     devices[nodeId]->metrics.incrementPacketSent();

}

void VisualiserManager::incrementListeningData(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Listening Data." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementListeningData();

}

void VisualiserManager::incrementTransmittingData(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Transmitting Data." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementTransmittingData();
}

void VisualiserManager::incrementListeningAck(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Listening Ack." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementListeningAck();
}

void VisualiserManager::incrementTransmittingAck(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Transmitting Ack." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementTransmittingAck();
}

void VisualiserManager::update(InputManager &inputManager)
{
    // Update animations and their life cycle
    {
        std::lock_guard<std::mutex> lock(broadcastAnimationsMutex);
        for (auto &animation : broadcastAnimations)
            animation->update();
        broadcastAnimations.erase(std::remove_if(broadcastAnimations.begin(), broadcastAnimations.end(),
                                                 [](const std::unique_ptr<BroadcastAnimation> &animation)
                                                 { return animation->isFinished(); }),
                                  broadcastAnimations.end());
    }

    {
        std::lock_guard<std::mutex> lock(dropAnimationsMutex);
        for (auto &drop : dropAnimations)
            drop->update();
        dropAnimations.erase(std::remove_if(dropAnimations.begin(), dropAnimations.end(),
                                            [](const std::unique_ptr<PacketDrop> &drop)
                                            { return drop->isFinished(); }),
                             dropAnimations.end());
    }

    {
        std::lock_guard<std::mutex> lock(arrowsMutex);
        for (auto &arrow : arrows)
            arrow->update();
        arrows.erase(std::remove_if(arrows.begin(), arrows.end(),
                                    [](const std::unique_ptr<Arrow> &arrow)
                                    { return arrow->isFinished() && arrow->isReceptionFinished(); }),
                     arrows.end());
    }

    {
        std::lock_guard<std::mutex> lock(receptionIconsMutex);
        receptionIcons.erase(std::remove_if(receptionIcons.begin(), receptionIcons.end(),
                                            [](const std::unique_ptr<ReceptionIcon> &icon)
                                            { return icon->isFinished(); }),
                             receptionIcons.end());
    }


    {
        std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        device->update(inputManager,gui, canvas);
    }
    }
}

void VisualiserManager::draw(sf::RenderWindow &window, sf::View &networkView, ProtocolVisualisationState &state)
{
        canvas->setView(networkView);
        canvas->clear(tgui::Color(30, 30, 30));

        communicationModeText->setText("Communication Mode: " + state.communicationMode);

        //Todo: have a tick duration in the config file
        timeText->setText("Time: " + std::to_string(state.tickNumber /* Config::TICK_DURATION*/) + "s");


        //todo: display the metrics in the UI
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


    if (routingDisplayEnabled)
        drawRootings(canvas);

    {
        std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        device->draw(canvas);
        }
    }

    {
        std::lock_guard<std::mutex> lock(arrowsMutex);
        for (auto &arrow : arrows)
            arrow->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(broadcastAnimationsMutex);
        for (auto &animation : broadcastAnimations)
            animation->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(receptionIconsMutex);
        for (auto &icon : receptionIcons)
            icon->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(dropAnimationsMutex);
        for (auto &animation : dropAnimations)
            animation->draw(canvas);
    }

    canvas->display();

}

void VisualiserManager::updateDevicesState(int nodeId, DeviceState state)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to update its state." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }

    devices[nodeId]->setState(state);

}

std::pair<sf::Vector2f, bool> VisualiserManager::findDeviceCoordinates(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to find device coordinate." << std::endl;
        return {{}, false};
    }

    return {devices[nodeId]->getCenteredPosition(), true};
}

void VisualiserManager::drawRootings(tgui::CanvasSFML::Ptr canvas)
{
    for (const auto &[device, connectedDevices] : routings)
    {

        for (const auto &connectedDevice : connectedDevices)
        {
            // get start position and end position
            sf::Vector2f start;
            sf::Vector2f end;
            bool foundPos = false;

            auto it1 = devices.find(device);
            auto it2 = devices.find(connectedDevice);

            if (it1 != devices.end() && it2 != devices.end()) {
                start = it1->second->getCenteredPosition();
                end = it2->second->getCenteredPosition();
                foundPos = true;
            }

            if (foundPos)
            {
                drawArrowWithHeads(canvas, start, end, 35.f);
            }
        }
    }
}
