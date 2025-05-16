#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

class InputManager {
public:
    void handleEvent(const std::optional<sf::Event>& event);
    void postUpdate(const sf::RenderWindow& window);

    sf::Vector2i getMouseScreenPosition() const;
    sf::Vector2f getMouseWorldPosition(const sf::RenderWindow& window) const;

    bool isKeyPressed(sf::Keyboard::Scancode key) const;
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    bool isMouseJustPressed(sf::Mouse::Button button) const;
    bool isDoubleClick(sf::Mouse::Button button) const;
    float getMouseWheelDelta() const; // Expose the accumulated delta


private:
    sf::Vector2i mouseScreenPosition{};
    std::unordered_set<sf::Keyboard::Scancode> pressedKeys;
    std::unordered_set<sf::Mouse::Button> pressedMouseButtons;
    std::unordered_set<sf::Mouse::Button> justPressedMouseButtons;
    std::unordered_map<sf::Mouse::Button, std::chrono::steady_clock::time_point> lastClickTime;

    static constexpr float doubleClickThresholdSec = 0.3f;
    float accumulatedMouseWheelDelta = 0.f; // Accumulate per frame

};
