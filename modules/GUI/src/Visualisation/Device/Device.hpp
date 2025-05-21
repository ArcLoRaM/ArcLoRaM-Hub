#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"

enum class DeviceState {
    Sleep,
    Listen,
    Transmit,
    Communicate
};

enum class DeviceClass {
    C1,
    C2,
    C3
};


class Device {
private:
    sf::CircleShape shape;
    sf::Texture* iconTexture = nullptr;

    std::optional<sf::Text> infoTextId;
    std::optional<sf::Text> infoTextBattery;
    std::optional<sf::Text> infoTextCoordinates;
    sf::RectangleShape infoWindow;

    std::string textId;
    std::string textBattery;
    std::string textCoordinates;


    sf::Font* font = nullptr;

    bool displayInfoWindow = false;
    bool isHovered = false;
    sf::Vector2f centeredPosition;

    int nodeId = 0;
    DeviceClass classNode ;
    double batteryLevel = 0;
    DeviceState state = DeviceState::Sleep;
    std::string getTextureKey(DeviceClass cls, DeviceState state);
public:

    Device(int nodeId, DeviceClass classNode, sf::Vector2f centeredPosition, double batteryLevel = 0);

    void updateCoordinatesString();

    void draw(sf::RenderWindow& window);
    void update(const InputManager& input);

    void setState(DeviceState newState);

    //return the position of the Shape (top left corner)
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }

    //return the centered position of the Shape (you want to use this one most of the time)
    sf::Vector2f getCenteredPosition() const {
        return centeredPosition;
    }

    int getNodeId() const {
        return nodeId;
    }

void changePosition(const sf::Vector2f& newCenteredPos) {
    centeredPosition = newCenteredPos;
    shape.setPosition(centeredPosition - sf::Vector2f(config::radiusIcon, config::radiusIcon));
    updateCoordinatesString();
}

    sf::Vector2f getSize() const {
        return shape.getGlobalBounds().size;
    }
    bool getIsHovered() const {
        return isHovered;
    }
};
