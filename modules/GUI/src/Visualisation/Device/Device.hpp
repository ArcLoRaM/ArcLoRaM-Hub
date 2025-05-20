#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"
class Device {
private:
    sf::CircleShape shape;
    sf::Texture* iconTexture = nullptr;

    std::optional<sf::Text> infoTextId;
    std::optional<sf::Text> infoTextBattery;
    sf::RectangleShape infoWindow;

    std::string textId;
    std::string textBattery;
    sf::Font* font = nullptr;

    bool displayInfoWindow = false;
    bool isHovered = false;

public:

    //should be private: Todo
    int nodeId = 0;
    int classNode = 0;
    double batteryLevel = 0;
    std::string state = "Sleep";

    std::pair<int, int> coordinates; 
    // why is it a pair? Should be a vector of two floats: Todo

    Device(int nodeId, int classNode, std::pair<int, int> coordinates, double batteryLevel = 0);

    void draw(sf::RenderWindow& window);
    void update(const InputManager& input);

    void changePNG(const std::string& state);

    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }
    void changePosition(const sf::Vector2f& newCenteredPos) {
        //Todo: get rid of Coordinates !!! It doesn't make any sense
        coordinates.first = static_cast<int>(newCenteredPos.x);
        coordinates.second = static_cast<int>(newCenteredPos.y);
        sf::Vector2f newPos = {newCenteredPos.x - config::radiusIcon, newCenteredPos.y - config::radiusIcon};
        shape.setPosition(newPos);
    }
    sf::Vector2f getSize() const {
        return shape.getGlobalBounds().size;
    }
    bool getIsHovered() const {
        return isHovered;
    }
};
