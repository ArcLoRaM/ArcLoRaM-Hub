#pragma once

#include <utility>
#include <SFML/Graphics.hpp>

class Device {

public:
    Device(int nodeId,int classNode, std::pair<int, int> coordinates,double batteryLevel=0);
    std::pair<int, int> coordinates;
    int nodeId=0;
    int classNode;
    double batteryLevel=0;

    sf::CircleShape shape;
    sf::Texture* iconTexture;    // Texture for the icon
    std::string state;

    // sf::RectangleShape shape;
    // sf::Sprite icon;    // Icon inside the button
    // sf::Texture iconTexture; // Texture for the icon
    void draw(sf::RenderWindow& window);
    void changePNG(std::string state);
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    // Info window
    sf::RectangleShape infoWindow;
    bool displayInfoWindow=false;

    std::optional< sf::Text> infoTextId;
     std::string textId;
     std::string textBattery;
     sf::Font* font; 
    std::optional< sf::Text> infoTextBattery;
};