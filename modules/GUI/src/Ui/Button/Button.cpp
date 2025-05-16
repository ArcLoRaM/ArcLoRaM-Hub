

#include "Button.hpp"
#include <iostream>
#include "../../Shared/RessourceManager/RessourceManager.hpp"


// Button::Button(sf::RenderWindow& window,float x, float y, float width, float height,
//     sf::Color color, std::string& stateRef,
//     const std::string& on, const std::string& off, const std::string& ressourceKey)
//     : window(window),state(stateRef), onState(on), offState(off) {

// }

Button::Button(float x, float y, float width, float height, sf::Color color, const std::string & resourceKey)
{
        shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(sf::Vector2f(x, y));
    shape.setFillColor(color);
    // Load the icon texture

    iconTexture = &ResourceManager::getInstance().getTexture(resourceKey);
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


//with text

Button::Button(float x, float y, float width, float height, sf::Color color, const std::string& labelText, const std::string& fontKey)
{
    shape.setSize(sf::Vector2f(width, height));
    shape.setPosition(sf::Vector2f(x, y));
    shape.setFillColor(color);

    font = &ResourceManager::getInstance().getFont(fontKey);

    text.emplace(*font, labelText, 30);
    text->setFillColor(sf::Color::White);

    sf::FloatRect textBounds = text->getLocalBounds();
    text->setOrigin({textBounds.position.x + textBounds.size.x / 2.f,
        textBounds.position.y + textBounds.size.y / 2.f}
        
    );
    text->setPosition({x + width / 2.f, y + height / 2.f} );
}




void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    if (icon) window.draw(*icon);
    if (text) window.draw(*text);
}

void Button::setOnClick(std::function<void()> callback) {
    onClickAction = std::move(callback);
}

void Button::update(const InputManager& input) {
    sf::Vector2f mouseWorldPosition = input.getMouseWorldPosition();

    bool wasHovered = isHovered;
    isHovered = shape.getGlobalBounds().contains(mouseWorldPosition);

    if (isHovered && !wasHovered) {
        shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 200));
    } else if (!isHovered && wasHovered) {
        shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 255));
    }

    if (isHovered && input.isMouseJustPressed()) {
        if (onClickAction) {
            onClickAction(); // Execute the assigned action
            std::cout << "onClickAction executed" << std::endl; 
        }
    }
}