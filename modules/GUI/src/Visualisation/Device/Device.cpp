#include "Device.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>
#include <magic_enum.hpp>



Device::Device(int nodeId,DeviceClass classNode, sf::Vector2f centeredPosition,int hopCount, double batteryLevel)
    : nodeId(nodeId), classNode(classNode), centeredPosition(centeredPosition),hopCount(hopCount), batteryLevel(batteryLevel) {

    shape = sf::CircleShape(config::radiusIcon);

    iconTexture= &ResourceManager::getInstance().getTexture(getTextureKey(classNode, state));
    shape.setTexture(iconTexture);
        shape.setPosition(sf::Vector2f(
            centeredPosition - sf::Vector2f(config::radiusIcon, config::radiusIcon)
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

void Device::draw(tgui::CanvasSFML::Ptr canvas) {
    if(!canvas) {
        std::cerr << "Canvas is null, cannot draw Device." << std::endl;
        return;
    }

    canvas->draw(shape);

    if (displayInfoWindow) {
        canvas->draw(infoWindow);
        canvas->draw(*infoTextId);
        canvas->draw(*infoTextBattery);
        canvas->draw(*infoTextCoordinates);
    }
}

void Device::update(const InputManager& input,const tgui::Gui& gui,const tgui::CanvasSFML::Ptr& canvas) {


        // 1. Get raw mouse position in window
    sf::Vector2i pixelPos = input.getMouseScreenPosition();  // <- your sf::RenderWindow

    // 2. Map to GUI layout space (e.g. %/px positioning)
    sf::Vector2f guiPos = gui.mapPixelToCoords(pixelPos);

    // 3. Subtract canvas position to get local canvas coords
    sf::Vector2f canvasLocal = guiPos - sf::Vector2f(canvas->getAbsolutePosition());

    // 4. Convert to view/world space inside the canvas
    sf::Vector2f worldCoords = canvas->mapPixelToCoords(canvasLocal);

    bool wasHovered = isHovered;
    isHovered = shape.getGlobalBounds().contains(worldCoords);

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
            infoWindow.setPosition(shape.getPosition()+shape.getRadius() * 2.f * sf::Vector2f(0.5f, 0.5f)+ sf::Vector2f(20, 20));
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