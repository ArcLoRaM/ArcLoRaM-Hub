

#include "Button.hpp"
#include <iostream>

Button::Button(float x, float y, float width, float height,
    sf::Color color, std::string& stateRef,
    const std::string& on, const std::string& off, const std::string& iconPath)
    : state(stateRef), onState(on), offState(off) {
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(x, y);
    shape.setFillColor(color);

    // Load the icon texture
    if (!iconTexture.loadFromFile(iconPath)) {
        throw std::runtime_error("Failed to load icon texture from " + iconPath);
    }

    // Set up the sprite
    icon.setTexture(iconTexture);

    // Scale the icon to fit inside the button if needed
    float scaleX = (width * 0.6f) / iconTexture.getSize().x;
    float scaleY = (height * 0.6f) / iconTexture.getSize().y;
    icon.setScale(scaleX, scaleY);

    // Center the icon inside the button
    icon.setPosition(
        x + (width - iconTexture.getSize().x * scaleX) / 2.f,
        y + (height - iconTexture.getSize().y * scaleY) / 2.f
    );
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(icon); // Draw the icon
}

void Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorldPos = window.mapPixelToCoords(mousePos);

        if (shape.getGlobalBounds().contains(mouseWorldPos)) {
            // Toggle state and update global variable
            std::cout<<"Button pressed, stateis "<<state<<std::endl;
            state = (state == offState) ? onState : offState;

        }
    }
}

std::string Button::getState() const {
    return state;
}
