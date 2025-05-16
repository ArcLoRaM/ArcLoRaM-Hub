#include "InputManager.hpp"
#include <iostream>

void InputManager::handleEvent(const std::optional<sf::Event>& event) {
    if (!event) return;

    if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        pressedKeys.insert(keyPressed->scancode);
    }
    else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
        pressedKeys.erase(keyReleased->scancode);
    }
    else if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (!pressedMouseButtons.contains(mousePressed->button)) {
            justPressedMouseButtons.insert(mousePressed->button);
            std::cout << "Mouse button pressed: " << static_cast<int>(mousePressed->button) << std::endl;
        }
        pressedMouseButtons.insert(mousePressed->button);
    }
    else if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
        pressedMouseButtons.erase(mouseReleased->button);
    }
    else if (const auto* wheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
        if (wheelScrolled->wheel == sf::Mouse::Wheel::Vertical) {
            accumulatedMouseWheelDelta += wheelScrolled->delta;
        }
    }
}

void InputManager::postUpdate(const sf::RenderWindow& window) {
    mouseScreenPosition = sf::Mouse::getPosition(window);
    mouseWorldPosition = window.mapPixelToCoords(mouseScreenPosition);

    justPressedMouseButtons.clear();  // Reset per-frame
    accumulatedMouseWheelDelta = 0.f;
}

sf::Vector2i InputManager::getMouseScreenPosition() const {
    return mouseScreenPosition;
}

sf::Vector2f InputManager::getMouseWorldPosition() const {
    return mouseWorldPosition;
}

bool InputManager::isKeyPressed(sf::Keyboard::Scancode key) const {
    return pressedKeys.contains(key);
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    return pressedMouseButtons.contains(button);
}

bool InputManager::isMouseJustPressed() const {
    return !justPressedMouseButtons.empty();
}

float InputManager::getMouseWheelDelta() const {
    return accumulatedMouseWheelDelta;
}
