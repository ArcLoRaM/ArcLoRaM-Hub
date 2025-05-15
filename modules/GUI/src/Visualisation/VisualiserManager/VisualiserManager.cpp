


#include "VisualiserManager.hpp"
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"

VisualiserManager::VisualiserManager(sf::RenderWindow& window)
    : window(window)
{

    font = &ResourceManager::getInstance().getFont("Arial");
    text.emplace(*font);
    text->setCharacterSize(20);
    text->setFillColor(sf::Color::White);


    communicationMode=sf::Text(*font);
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
    nbRetransmissionString="Retransmissions:  ";
    
    energyExpenditure.emplace(*font);
    energyExpenditure->setCharacterSize(20);
    sf::Vector2f position4(10.f, 90.f); 
    energyExpenditure->setPosition(position4);
    energyExpenditure->setFillColor(sf::Color::White); 
    energyExpenditureString="Energy Expenditure: ";
          

    std::unique_ptr<Button> buttonRouting =std::make_unique <Button> (window,x4coor, y4coor, width4, height4, color4, state4, on, off,"Rooting_Button");
    addButton(std::move(buttonRouting));

}

void VisualiserManager::addButton(std::unique_ptr<Button> button) {
	buttons.push_back(std::move(button));
}

void VisualiserManager::addDevice(std::unique_ptr<Device> device) {
    devices.push_back(std::move(device));
}

void VisualiserManager::addArrow(std::unique_ptr<Arrow> arrow)
{
    arrows.push_back(std::move(arrow));
}

void VisualiserManager::addReceptionIcon(std::unique_ptr<ReceptionIcon> receptionIcon)
{
    receptionIcons.push_back(std::move(receptionIcon));
}

void VisualiserManager::addDropAnimation(std::unique_ptr<PacketDrop> dropAnimation)
{
    dropAnimations.push_back(std::move(dropAnimation));
}

void VisualiserManager::changeArrowState(int senderId, int receiverId, std::string state)
{
    if(state!="interference"&&state!="notListening"&&state!="received"){
        throw std::runtime_error("Error: state not recognized");
    }
    for(auto& arrow : arrows){
        if(arrow->SenderId==senderId && arrow->ReceiverId==receiverId){
            arrow->receptionState=state;
        }
    }
}

void VisualiserManager::startBroadcast(const sf::Vector2f &startPosition, float duration)
{
        broadcastAnimations.push_back(std::make_unique<BroadcastAnimation>(startPosition, duration));

}

void VisualiserManager::addDeviceId(int id)
{
    if (devicesId.emplace(id).second) {
            routings[id] = {};
        } else {
            std::cout << "********Device " << id << " already exists********.\n";
        }
}

void VisualiserManager::addRouting(int id1, int id2)
{
    // ID1 ---> ID2 , path towards ID2

    if (devicesId.count(id1) && devicesId.count(id2)) {
            routings[id1].insert(id2);  // Only store id2 in id1's adjacency list       
        } else {
            std::cout << "********One or both devices do not exist.********\n";
    }
}

void VisualiserManager::removeRouting(int id1, int id2)
{
    if (devicesId.count(id1) && devicesId.count(id2)) {
            routings[id1].erase(id2);
        }else {
            std::cout << "********One or both devices do not exist.********\n";
        }
}

void VisualiserManager::update()
{
    // Update all broadcast animations
    for (auto& animation : broadcastAnimations) {
        animation->update();
    }

    //remove finished broadcast animations
    broadcastAnimations.erase(std::remove_if(broadcastAnimations.begin(), broadcastAnimations.end(),
        [](const std::unique_ptr<BroadcastAnimation>& animation) {
            return animation->isFinished();
        }),
        broadcastAnimations.end());

    // Update all drop animations
    for (auto& drop : dropAnimations) {
        drop->update();
    }

    //remove finished drop animations
    dropAnimations.erase(std::remove_if(dropAnimations.begin(), dropAnimations.end(),
        [](const std::unique_ptr<PacketDrop>& drop) {
            return drop->isFinished();
        }),
        dropAnimations.end());

    

    //Update transmission animation
    for(auto& arrow : arrows){
        arrow->update();
    }

    //remove finished arrows
    arrows.erase(std::remove_if(arrows.begin(), arrows.end(), [](const std::unique_ptr<Arrow>& arrow) {
        return (arrow->isFinished()&&arrow->isReceptionFinished());
    }), arrows.end());

    //remove finished receptionIcons
    receptionIcons.erase(std::remove_if(receptionIcons.begin(), receptionIcons.end(), [](const std::unique_ptr<ReceptionIcon>& icon) {
        return (icon->isFinished());
    }), receptionIcons.end());

}
void VisualiserManager::draw(sf::RenderWindow& window) {
    std::ostringstream displayText;

if(buttons[0]->getState()=="ON")    drawRootings(window);


    for (auto& device : devices) {
        device->draw(window);
    }

    for(auto& arrow : arrows){
        arrow->draw(window);
    }
    for(auto& animation : broadcastAnimations){
        animation->draw(window);
    }
    for(auto& recepIcon :receptionIcons){
        recepIcon->draw(window);
    }

    for(auto& animation: dropAnimations)
    {
        animation->draw(window);
    }



    //Draw Metrics and titles using Default View

    window.setView(window.getDefaultView());
    communicationMode->setString("Communication Mode: "+COMMUNICATION_MODE);
    window.draw(*communicationMode);

    tickNb->setString("Tick Number: " + std::to_string(TICK_NB));
    window.draw(*tickNb);

    energyExpenditure->setString(energyExpenditureString+std::to_string(ENERGYEXP));
    window.draw(*energyExpenditure);

    nbRetransmission->setString(nbRetransmissionString+std::to_string(NBRETRANSMISSION));
    window.draw(*nbRetransmission);

}


void VisualiserManager::drawRootings(sf::RenderWindow& window)
{
    for (const auto& [device, connectedDevices] : routings) {
            
            for (const auto& connectedDevice : connectedDevices) {
                //get start position and end position
                sf::Vector2f start;
                sf::Vector2f end;
                bool foundPos=false;
                for(auto& device1 : devices){
                    if(device1->nodeId==device){
                         start=device1->shape.getPosition();
                         //center the start position
                         start.x+=radiusIcon;
                            start.y+=radiusIcon;
                        //get end position
                        for(auto& device2 : devices){
                            if(device2->nodeId==connectedDevice){
                                 end=device2->shape.getPosition();
                                 //center the end position
                                 end.x+=radiusIcon;
                                    end.y+=radiusIcon;
                                 foundPos=true;
                            }
                        }
                    }
                }

                if(foundPos){
                    drawArrowWithHeads(window, start, end, 35.f);
                }
            }
        }

}


