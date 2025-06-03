#include "TypeableInput.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include <cctype>
#include <iostream>

TypeableInput::TypeableInput() {
    displayText.emplace(ResourceManager::getInstance().getFont("Arial"), "", 24);
    displayText->setFillColor(sf::Color::White);
    displayText->setPosition({500.f, 50.f});
    displayText->setString("Enter filename: ");
    
    shape.setFillColor(sf::Color(50, 50, 50, 255));
    shape.setSize(sf::Vector2f(150.f, 50.f));
    shape.setPosition(sf::Vector2f(500.f, 50.f));

}

void TypeableInput::setSelected(bool selected) {
    isSelected = selected;
    if (!selected) {
        cursorVisible = false;
    } else {
        cursorTimer.restart(); // reset blinking when reselected
        cursorVisible = true;
    }
}

void TypeableInput::update(InputManager& inputManager) {
    sf::Vector2f mousePos = inputManager.getMouseUIScreenPosition();  
    std::cout << "Typed Updated"<<std::endl;
    bool wasHovered = isHovered;
    isHovered = shape.getGlobalBounds().contains(mousePos);    
   if (isHovered && !wasHovered) {
        shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 200));
    } else if (!isHovered && wasHovered) {
        shape.setFillColor(sf::Color(shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, 255));
    }
    if (isHovered && inputManager.isMouseJustPressed()){
        setSelected(true);
         std::cout << "Typed Selected"<<std::endl;
    } else if (!isHovered && inputManager.isMouseJustPressed()) {
        setSelected(false);
    }
    // Determine final color based on hover and push state
    // Apply hover effect with transparency only, preserving base color
    sf::Color baseColor = shape.getFillColor();
    uint8_t alpha = (isHovered || isSelected) ? 200 : 255;
    baseColor.a = alpha;
    shape.setFillColor(baseColor);


    if (!isSelected) return;

    for (char32_t unicode : inputManager.getTextBuffer()) {
        if (unicode == 8) { // Backspace
            if (!inputString.empty())
                inputString.pop_back();
        } else if (unicode == '\r' || unicode == '\n') {
            // Enter key logic (optional)
        } else if (std::isalnum(static_cast<unsigned char>(unicode)) || unicode == '_' || unicode == '-' || unicode == '.') {
            inputString += static_cast<char>(unicode);
        }
    }
    inputManager.clearTextBuffer();

    // Blink cursor
    if (cursorTimer.getElapsedTime().asSeconds() > cursorBlinkInterval) {
        cursorVisible = !cursorVisible;
        cursorTimer.restart();
    }

    std::string displayStr = "Enter filename: " + inputString;
    if (isSelected && cursorVisible)
        displayStr += "|";

    displayText->setString(displayStr);

}

void TypeableInput::draw(sf::RenderWindow& window) const {
    window.draw(*displayText);
}

const std::string& TypeableInput::getText() const {
    return inputString;
}
