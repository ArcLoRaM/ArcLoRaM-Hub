#include "InputManager.hpp"
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
            auto now = std::chrono::steady_clock::now();
            lastClickTime[mousePressed->button] = now;
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
    justPressedMouseButtons.clear();
    accumulatedMouseWheelDelta = 0.f; // Clear wheel delta after frame
}

float InputManager::getMouseWheelDelta() const {
    return accumulatedMouseWheelDelta;
}


sf::Vector2i InputManager::getMouseScreenPosition() const {
    return mouseScreenPosition;
}

sf::Vector2f InputManager::getMouseWorldPosition(const sf::RenderWindow& window) const {
    return window.mapPixelToCoords(mouseScreenPosition);
}

bool InputManager::isKeyPressed(sf::Keyboard::Scancode key) const {
    return pressedKeys.contains(key);
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    return pressedMouseButtons.contains(button);
}

bool InputManager::isMouseJustPressed(sf::Mouse::Button button) const {
    return justPressedMouseButtons.contains(button);
}

bool InputManager::isDoubleClick(sf::Mouse::Button button) const {
    if (!lastClickTime.contains(button)) return false;
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration<float>(now - lastClickTime.at(button)).count();
    return duration <= doubleClickThresholdSec;
}
