#include "Device.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>
#include <magic_enum.hpp>

Device::Device(int nodeId,DeviceClass classNode, sf::Vector2f centeredPosition, double batteryLevel)
    : nodeId(nodeId), classNode(classNode), centeredPosition(centeredPosition), batteryLevel(batteryLevel) {

    shape = sf::CircleShape(config::radiusIcon);

    iconTexture= &ResourceManager::getInstance().getTexture(getTextureKey(classNode, state));
    shape.setTexture(iconTexture);
        shape.setPosition(sf::Vector2f(
            centeredPosition - sf::Vector2f(config::radiusIcon/2, config::radiusIcon/2)
        ));


    font = &ResourceManager::getInstance().getFont("Arial");

    infoWindow.setSize({150, 80});
    infoWindow.setFillColor(sf::Color(0, 0, 0, 200));
    infoWindow.setOutlineThickness(2);
    infoWindow.setOutlineColor(sf::Color::White);

    textId = "Node ID: " + std::to_string(nodeId);
    infoTextId = sf::Text(*font);
    infoTextId->setCharacterSize(14);
    infoTextId->setString(textId);
    infoTextId->setFillColor(sf::Color::White);

    textBattery = "Battery: " + std::to_string(batteryLevel);
    infoTextBattery = sf::Text(*font);
    infoTextBattery->setCharacterSize(14);
    infoTextBattery->setString(textBattery);
    infoTextBattery->setFillColor(sf::Color::White);

    updateCoordinatesString();
    infoTextCoordinates = sf::Text(*font);
    infoTextCoordinates->setCharacterSize(14);
    infoTextCoordinates->setString(textCoordinates);
    infoTextCoordinates->setFillColor(sf::Color::White);
}

void Device::updateCoordinatesString()
{
    std::ostringstream ss;
    ss << "Coor: (" << static_cast<int>(getCenteredPosition().x) << ", " << static_cast<int>(getCenteredPosition().y) << ")";
    textCoordinates = ss.str();
}

void Device::draw(sf::RenderWindow& window) {
    window.draw(shape);

    if (displayInfoWindow) {
        window.draw(infoWindow);
        window.draw(*infoTextId);
        window.draw(*infoTextBattery);
        window.draw(*infoTextCoordinates);
    }
}

void Device::update(const InputManager& input) {
    sf::Vector2f mouseWorldPos = input.getMouseWorldPosition();
    bool wasHovered = isHovered;
    isHovered = shape.getGlobalBounds().contains(mouseWorldPos);

    // Visual feedback (hover effect)
    if (isHovered && !wasHovered) {
        shape.setScale(sf::Vector2f{1.15f, 1.15f}); // Slight zoom on hover
    } else if (!isHovered && wasHovered) {
        shape.setScale(sf::Vector2f{1.f, 1.f}); // Reset scale
    }

    // DoubleClick to toggle info window
    if (isHovered && input.isMouseDoubleClicked(sf::Mouse::Button::Left)) {
        displayInfoWindow = !displayInfoWindow;

        if (displayInfoWindow) {
            infoWindow.setPosition(mouseWorldPos + sf::Vector2f(10, 10));
            infoTextId->setPosition(infoWindow.getPosition() + sf::Vector2f(10, 10));
            infoTextBattery->setPosition(infoWindow.getPosition() + sf::Vector2f(10, 30));
            infoTextCoordinates->setPosition(infoWindow.getPosition() + sf::Vector2f(10, 50));
        }
    }
}

void Device::setState(DeviceState newState) {
    state = newState;

    std::string key = getTextureKey(classNode, state);
    iconTexture = &ResourceManager::getInstance().getTexture(key);
    shape.setTexture(iconTexture);
}


std::string Device::getTextureKey(DeviceClass cls, DeviceState state) {
    return std::string(magic_enum::enum_name(cls)) + "_" + std::string(magic_enum::enum_name(state));
}