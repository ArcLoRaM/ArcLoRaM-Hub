

#include "Button.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"

Button::Button(sf::RenderWindow& window,float x, float y, float width, float height,
    sf::Color color, std::string& stateRef,
    const std::string& on, const std::string& off, const std::string& ressourceKey)
    : window(window),state(stateRef), onState(on), offState(off) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(sf::Vector2f(x, y));
    shape.setFillColor(color);
    // Load the icon texture

    iconTexture = &ResourceManager::getInstance().getTexture(ressourceKey);
    // Set up the sprite
    icon = std::make_optional<sf::Sprite>(*iconTexture);

    // Scale the icon to fit inside the button if needed
    float scaleX = (width * 0.6f) / iconTexture->getSize().x;
    float scaleY = (height * 0.6f) / iconTexture->getSize().y;
    icon->setScale(sf::Vector2f(scaleX, scaleY));

    // Center the icon inside the button
    icon->setPosition(sf::Vector2f(
        x + (width - iconTexture->getSize().x * scaleX) / 2.f,
        y + (height - iconTexture->getSize().y * scaleY) / 2.f
    ));
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(*icon); // Draw the icon
}

void Button::handleEvent(const sf::Event& event) {

    if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseButtonPressed->button == sf::Mouse::Button::Left)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);
    
            if (shape.getGlobalBounds().contains(mouseWorldPos)) {
                // Toggle state and update global variable
                std::cout<<"Button pressed, stateis "<<state<<std::endl;
                state = (state == offState) ? onState : offState;
    
            }
        }
    }
}

std::string Button::getState() const {
    return state;
}
