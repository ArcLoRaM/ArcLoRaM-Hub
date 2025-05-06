



#include "Device.hpp"
#include <iostream>
#include "../Common/Common.hpp"

Device::Device(int nodeId,
    int classNode, std::pair<int, int> coordinates,double batteryLevel):
    nodeId(nodeId), classNode(classNode), coordinates(coordinates),batteryLevel(batteryLevel) {

       

        shape= sf::CircleShape (radiusIcon);
        iconTexture= sf::Texture();

        state="Sleep";

        //depending of the class, we load the corresponding icon
        if(classNode==3){
            if (!iconTexture.loadFromFile("assets/Icons/C3_Sleep.png")) {
                throw std::runtime_error("Failed to load icon texture for class 3·");
            }            
            shape.setTexture(&iconTexture);
            shape.setPosition(
                coordinates.first- radiusIcon ,
                coordinates.second- radiusIcon
            );

        }
        else if(classNode==2){
            if (!iconTexture.loadFromFile("assets/Icons/C2_Sleep.png")) {
                throw std::runtime_error("Failed to load icon texture for class 2·");
            }            
            shape.setTexture(&iconTexture);
            shape.setPosition(
                coordinates.first- radiusIcon ,
                coordinates.second- radiusIcon
            );
        }
        else{
            std::cerr<<"Error: classNode not recognized"<<std::endl;
        }
        
        if (!font.loadFromFile("assets/arial.ttf")) {
            return; // Handle error
        }
        //Info WIndow
        infoWindow.setSize(sf::Vector2f(150, 80));
        infoWindow.setFillColor(sf::Color(0, 0, 0, 200)); // Semi-transparent black
        infoWindow.setOutlineThickness(2);
        infoWindow.setOutlineColor(sf::Color::White);
        textId="Node ID:"+std::to_string(nodeId);
        infoTextId.setCharacterSize(14);
        infoTextId.setFont(font);
        infoTextId.setString(textId);
        infoTextId.setFillColor(sf::Color::White);
        textBattery="Battery: "+std::to_string(batteryLevel);
        infoTextBattery.setCharacterSize(14);
        infoTextBattery.setFont(font);
        infoTextBattery.setString(textBattery);
        infoTextBattery.setFillColor(sf::Color::White);
        


}

void Device::draw(sf::RenderWindow &window)
{
    window.draw(shape);
    if(displayInfoWindow){
        window.draw(infoWindow);
        window.draw(infoTextId);
        window.draw(infoTextBattery);
    }
    //window.draw(icon);
}

void Device::changePNG(std::string state)
{
    std::string classNodeString;
    if(classNode==2){
        classNodeString="C2";
    }
    else if(classNode==3){
        classNodeString="C3";
    }
    else if(classNode==1){
        classNodeString="C1";
    }
    else{
        std::cerr<<"Error: classNode not recognized"<<std::endl;
    }

   std::string path = "assets/Icons/" +classNodeString+"_"+ state + ".png";
    if (!iconTexture.loadFromFile(path)) {
        throw std::runtime_error("Failed to load icon texture");
    }
    shape.setTexture(&iconTexture);
}

void Device::handleEvent(const sf::Event &event, const sf::RenderWindow &window)
{
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);

        if (shape.getGlobalBounds().contains(mouseWorldPos)){
            displayInfoWindow=!displayInfoWindow;
            if(displayInfoWindow){
                infoWindow.setPosition(mouseWorldPos + sf::Vector2f(10, 10));
                infoTextId.setPosition(infoWindow.getPosition() + sf::Vector2f(10, 10));
                infoTextBattery.setPosition(infoWindow.getPosition() + sf::Vector2f(10, 30));
            }
        }
    }
}
