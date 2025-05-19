#include "Device.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>

Device::Device(int nodeId, int classNode, std::pair<int, int> coordinates, double batteryLevel)
    : nodeId(nodeId), classNode(classNode), coordinates(coordinates), batteryLevel(batteryLevel) {

    shape = sf::CircleShape(config::radiusIcon);

    std::string classNodeString = (classNode == 2) ? "C2" : (classNode == 3) ? "C3" : "Unknown";
    if (classNodeString != "Unknown") {
        iconTexture = &ResourceManager::getInstance().getTexture(classNodeString + "_Sleep");
        shape.setTexture(iconTexture);
        shape.setPosition(sf::Vector2f(
            static_cast<float>(coordinates.first - config::radiusIcon),
            static_cast<float>(coordinates.second - config::radiusIcon)
        ));
    } else {
        std::cerr << "Error: classNode not recognized" << std::endl;
    }

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
}

void Device::draw(sf::RenderWindow& window) {
    window.draw(shape);

    if (displayInfoWindow) {
        window.draw(infoWindow);
        window.draw(*infoTextId);
        window.draw(*infoTextBattery);
    }
}

void Device::update(const InputManager& input) {
    sf::Vector2f mouseWorldPos = input.getMouseWorldPosition();
    bool wasHovered = isHovered;
    isHovered = shape.getGlobalBounds().contains(mouseWorldPos);

    // Visual feedback (hover effect)
    if (isHovered && !wasHovered) {
        shape.setScale(sf::Vector2f{1.f, 1.f}); // Slight zoom on hover
    } else if (!isHovered && wasHovered) {
        shape.setScale(sf::Vector2f{1.f, 1.f}); // Reset scale
    }

    // Click to toggle info window
    if (isHovered && input.isMouseJustPressed()) {
        displayInfoWindow = !displayInfoWindow;

        if (displayInfoWindow) {
            infoWindow.setPosition(mouseWorldPos + sf::Vector2f(10, 10));
            infoTextId->setPosition(infoWindow.getPosition() + sf::Vector2f(10, 10));
            infoTextBattery->setPosition(infoWindow.getPosition() + sf::Vector2f(10, 30));
        }
    }
}

void Device::changePNG(const std::string& newState) {
    state = newState;

    std::string classNodeString = (classNode == 2) ? "C2" : (classNode == 3) ? "C3" : "Unknown";
    if (classNodeString != "Unknown") {
        iconTexture = &ResourceManager::getInstance().getTexture(classNodeString + "_" + state);
        shape.setTexture(iconTexture);
    } else {
        std::cerr << "Error: classNode not recognized" << std::endl;
    }
}

