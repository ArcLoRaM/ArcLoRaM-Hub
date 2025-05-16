#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <optional>

class InputManager {
public:
    void handleEvent(const std::optional<sf::Event>& event);
    void postUpdate(const sf::RenderWindow& window);

    sf::Vector2i getMouseScreenPosition() const;
    sf::Vector2f getMouseWorldPosition() const;
    sf::Vector2f getMouseUIScreenPosition() const;

    bool isKeyPressed(sf::Keyboard::Scancode key) const;
    bool isMouseButtonPressed(sf::Mouse::Button button) const;

    // New simplified interface
    bool isMouseJustPressed() const;

    float getMouseWheelDelta() const;

private:
//you should have only two mouse positin? Check
    sf::Vector2i mouseScreenPosition{};
    sf::Vector2f mouseWorldPosition{};
    sf::Vector2f mouseUIScreenPosition{};  // <-- Add this

    std::unordered_set<sf::Keyboard::Scancode> pressedKeys;
    std::unordered_set<sf::Mouse::Button> pressedMouseButtons;
    std::unordered_set<sf::Mouse::Button> justPressedMouseButtons;

    float accumulatedMouseWheelDelta = 0.f;
};
