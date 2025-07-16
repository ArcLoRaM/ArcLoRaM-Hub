

#include "VisualiserManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/Helper.hpp"
#include "CsvMetricWriter.hpp"

VisualiserManager::VisualiserManager(ProtocolVisualisationState &state)

{

    font = &ResourceManager::getInstance().getFont("Arial");
    text.emplace(*font);
    text->setCharacterSize(20);
    text->setFillColor(sf::Color::White);

    communicationMode = sf::Text(*font);
    communicationMode->setCharacterSize(40);
    sf::Vector2f position1(10.f, 30.f);
    communicationMode->setPosition(position1);
    communicationMode->setFillColor(sf::Color::White);

    nbRetransmission.emplace(*font);
    nbRetransmission->setCharacterSize(20);
    sf::Vector2f position3(10.f, 80.f);
    nbRetransmission->setPosition(position3);
    nbRetransmission->setFillColor(sf::Color::White);
    nbRetransmissionString = "Retransmissions:  ";

    energyExpenditure.emplace(*font);
    energyExpenditure->setCharacterSize(20);
    sf::Vector2f position4(10.f, 110.f);
    energyExpenditure->setPosition(position4);
    energyExpenditure->setFillColor(sf::Color::White);
    energyExpenditureString = "Energy Expenditure: ";

    tickNb.emplace(*font);
    tickNb->setCharacterSize(20);
    sf::Vector2f position2(10.f, 150.f);
    tickNb->setPosition(position2);
    tickNb->setFillColor(sf::Color::White);

    pdrText.emplace(*font);
    pdrText->setCharacterSize(20);
    sf::Vector2f position5(10.f, 180.f);
    pdrText->setPosition(position5);
    pdrText->setFillColor(sf::Color::White);
    pdrString = "PDR: ";

    const float x4coor = 1800.f - 350.f;
    const float y4coor = 100.f;
    const float width4 = 80.f;
    const float height4 = 40;
    const sf::Color color4 = sf::Color::Yellow;

    auto buttonRouting = std::make_unique<Button>(x4coor, y4coor, width4, height4, color4, "Rooting_Button");
    buttonRouting->setOnClick([this]()
                              {
                                  // Toggle internal state (could be a bool, enum, etc.)
                                  routingDisplayEnabled = !routingDisplayEnabled;
                                  // std::cout << "Routing display toggled: " << (routingDisplayEnabled ? "ON" : "OFF") << std::endl;
                              });

    buttons.push_back(std::move(buttonRouting));

    const float y5coor = 160.f;
    const sf::Color color5 = sf::Color::Green;

    auto buttonSave = std::make_unique<Button>(x4coor, y5coor, width4, height4, color5, "Save", "Arial", false);
    buttonSave->setOnClick([this, &state]()
                           {
            try {
        CsvMetricWriter writer;
        writer.writeNetworkMetricsToCsv("output/network_state.csv", *this, state);
        std::cout << "Network state saved to network_state.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "Error writing CSV: " << e.what() << '\n';
    } });

    buttons.push_back(std::move(buttonSave));
}

void VisualiserManager::addButton(std::unique_ptr<Button> button)
{
    std::lock_guard<std::mutex> lock(buttonsMutex);
    buttons.push_back(std::move(button));
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

    // with interactivity through inputManager Injection
    {
        std::lock_guard<std::mutex> lock(buttonsMutex);
        for (auto &button : buttons)
            button->update(inputManager);
    }

    {
        std::lock_guard<std::mutex> lock(devicesMutex);
for (auto& [id, device] : devices) {
    // device->update(inputManager);
}
    }
}

void VisualiserManager::draw(sf::RenderWindow &window, sf::View &networkView, ProtocolVisualisationState &state)
{

    // Todo: have their own mutexes
    window.setView(window.getDefaultView());
    communicationMode->setString("Communication Mode: " + state.communicationMode);
    window.draw(*communicationMode);

    tickNb->setString("Tick Number: " + std::to_string(state.tickNumber));
    window.draw(*tickNb);

    energyExpenditure->setString(energyExpenditureString + std::to_string(state.energyExp));
    window.draw(*energyExpenditure);

    nbRetransmission->setString(nbRetransmissionString + std::to_string(state.retransmissions));
    window.draw(*nbRetransmission);

    pdrText->setString(pdrString +  std::to_string(state.totalDataPacketsSent > 0 ? static_cast<float>(state.totalDataPacketsSent -state.retransmissions) / state.totalDataPacketsSent * 100 : 0) + "%");
    window.draw(*pdrText);
    {
        std::lock_guard<std::mutex> lock(buttonsMutex);
        for (auto &button : buttons)
        {
            button->draw(window);
        }
    }

    // Draw logs and also get rid of the oldest log messages if the limit is reached
    {
        std::lock_guard<std::mutex> lock(state.logMutex);

        float y = 940.0f;

        if (state.logMessages.size() > 10)
        {
            state.logMessages.erase(state.logMessages.begin(), state.logMessages.end() - 10);
        }

        for (auto it = state.logMessages.rbegin(); it != state.logMessages.rend(); ++it)
        {
            sf::Text text(*font, *it, 10);
            text.setFillColor(sf::Color::White);
            text.setPosition(sf::Vector2f(10.0f, y));
            window.draw(text);
            y -= 15.0f;
        }
    }

    window.setView(networkView);

    if (routingDisplayEnabled)
        // drawRootings(window);

    {
        std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto& [id, device] : devices) {
        // device->draw(window);
    }
}

    {
        std::lock_guard<std::mutex> lock(arrowsMutex);
        for (auto &arrow : arrows)
            // arrow->draw(window);
            auto a=2;
    }

    {
        std::lock_guard<std::mutex> lock(broadcastAnimationsMutex);
        for (auto &animation : broadcastAnimations)
            // animation->draw(window);
            auto a=2;
    }

    {
        std::lock_guard<std::mutex> lock(receptionIconsMutex);
        for (auto &icon : receptionIcons)
            icon->draw(window);
    }

    {
        std::lock_guard<std::mutex> lock(dropAnimationsMutex);
        for (auto &animation : dropAnimations)
            animation->draw(window);
    }

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

void VisualiserManager::drawRootings(sf::RenderWindow &window)
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
                // drawArrowWithHeads(window, start, end, 35.f);
            }
        }
    }
}
