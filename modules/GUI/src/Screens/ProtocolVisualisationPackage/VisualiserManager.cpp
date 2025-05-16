

#include "VisualiserManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"


VisualiserManager::VisualiserManager()

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

    tickNb.emplace(*font);
    tickNb->setCharacterSize(20);
    sf::Vector2f position2(10.f, 120.f);
    tickNb->setPosition(position2);
    tickNb->setFillColor(sf::Color::White);

    nbRetransmission.emplace(*font);
    nbRetransmission->setCharacterSize(20);
    sf::Vector2f position3(10.f, 60.f);
    nbRetransmission->setPosition(position3);
    nbRetransmission->setFillColor(sf::Color::White);
    nbRetransmissionString = "Retransmissions:  ";

    energyExpenditure.emplace(*font);
    energyExpenditure->setCharacterSize(20);
    sf::Vector2f position4(10.f, 90.f);
    energyExpenditure->setPosition(position4);
    energyExpenditure->setFillColor(sf::Color::White);
    energyExpenditureString = "Energy Expenditure: ";

    const float x4coor = 1800.f - 350.f;
    const float y4coor = 100.f;
    const float width4 = 80.f;
    const float height4 = 40;
    const sf::Color color4 = sf::Color::Yellow;

    // // Todo: change how you handle the state of the button - boolean, not better?
    // std::unique_ptr<Button> buttonRouting = std::make_unique<Button>(window, x4coor, y4coor, width4, height4, color4, state4, on, off, "Rooting_Button");
    // addButton(std::move(buttonRouting));
}

void VisualiserManager::addButton(std::unique_ptr<Button> button)
{
    std::lock_guard<std::mutex> lock(buttonsMutex);
    buttons.push_back(std::move(button));
}

void VisualiserManager::addDevice(std::unique_ptr<Device> device)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    devices.push_back(std::move(device));
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

void VisualiserManager::addDeviceId(int id)
{
    std::lock_guard<std::mutex> lock(devicesMutex);

    if (devicesId.emplace(id).second)
    {
        routings[id] = {};
    }
    else
    {
        std::cout << "********Device " << id << " already exists********.\n";
    }
}

void VisualiserManager::addRouting(int id1, int id2)
{
    // ID1 ---> ID2 , path towards ID2
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (devicesId.count(id1) && devicesId.count(id2))
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
    if (devicesId.count(id1) && devicesId.count(id2))
    {
        routings[id1].erase(id2);
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
    }
}
void VisualiserManager::update()
{
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
}

void VisualiserManager::draw(sf::RenderWindow &window, sf::View &networkView, const ProtocolVisualisationState &state)
{

    window.setView(window.getDefaultView());

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

    {
        std::lock_guard<std::mutex> lock(buttonsMutex);
        for (auto &button : buttons)
        {
            button->draw(window);
        }
    }

    // Draw logs (temporary retained from your code)
    {
        std::lock_guard<std::mutex> lock(state.logMutex);

        float y = 940.0f;

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

    if (buttons[0]->getState() == "ON")
        drawRootings(window);

    {
        std::lock_guard<std::mutex> lock(devicesMutex);
        for (auto &device : devices)
            device->draw(window);
    }

    {
        std::lock_guard<std::mutex> lock(arrowsMutex);
        for (auto &arrow : arrows)
            arrow->draw(window);
    }

    {
        std::lock_guard<std::mutex> lock(broadcastAnimationsMutex);
        for (auto &animation : broadcastAnimations)
            animation->draw(window);
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

void VisualiserManager::updateDevicesState(int nodeId, std::string state)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (auto &device : devices)
    {
        if (device->nodeId == nodeId)
        {
            device->state = state;
            device->changePNG(state);
        }
    }
}

std::pair<sf::Vector2f, bool> VisualiserManager::findDeviceCoordinates(int nodeId)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    for (const auto &device : devices)
    {
        if (device->nodeId == nodeId)
        {

            return {sf::Vector2f(static_cast<float>(device->coordinates.first), static_cast<float>(device->coordinates.second)), true};
        }
    }
    return {{}, false};
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
            for (auto &device1 : devices)
            {
                if (device1->nodeId == device)
                {
                    start = device1->shape.getPosition();
                    // center the start position
                    start.x += config::radiusIcon;
                    start.y += config::radiusIcon;
                    // get end position
                    for (auto &device2 : devices)
                    {
                        if (device2->nodeId == connectedDevice)
                        {
                            end = device2->shape.getPosition();
                            // center the end position
                            end.x += config::radiusIcon;
                            end.y += config::radiusIcon;
                            foundPos = true;
                        }
                    }
                }
            }

            if (foundPos)
            {
                drawArrowWithHeads(window, start, end, 35.f);
            }
        }
    }
}
