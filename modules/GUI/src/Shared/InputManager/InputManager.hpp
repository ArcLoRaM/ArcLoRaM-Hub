#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <unordered_set>
#include <chrono>

class InputManager {
public:
    void update(const sf::RenderWindow& window, const sf::Event& event);
    void postUpdate(const sf::RenderWindow& window); // Called once per frame after events

    sf::Vector2i getMousePosition() const;
    sf::Vector2f getMousePositionWorld(const sf::RenderWindow& window) const;
    bool isKeyPressed(sf::Keyboard::Scancode key) const;
    bool isMouseButtonPressed(sf::Mouse::Button button) const;
    bool isMouseJustPressed(sf::Mouse::Button button) const; // Single-frame detection
    bool isDoubleClick(sf::Mouse::Button button) const;

private:
    sf::Vector2i mouseScreenPosition{};
    std::unordered_set<sf::Keyboard::Scancode> pressedKeys;
    std::unordered_set<sf::Mouse::Button> pressedMouseButtons;
    std::unordered_set<sf::Mouse::Button> justPressedMouseButtons;
    std::unordered_map<sf::Mouse::Button, std::chrono::steady_clock::time_point> lastClickTime;

    static constexpr float doubleClickThresholdSec = 0.3f; // Double-click threshold
};

