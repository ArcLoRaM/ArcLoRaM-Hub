#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "../../Shared/InputManager/InputManager.hpp"

class TypeableInput {
public:
    TypeableInput();

    void setSelected(bool selected);
    void update( InputManager& inputManager);
    void draw(sf::RenderWindow& window) const;
    const std::string& getText() const;

private:
    std::string inputString;
    std::optional<sf::Text> displayText;
    bool isSelected = false;

    // Blinking cursor
    sf::Clock cursorTimer;
    bool cursorVisible = true;
    float cursorBlinkInterval = 0.5f; // seconds

    sf::RectangleShape shape;
    bool isHovered = false;
};
