// #include "InputManager.hpp"

// void InputManager::update(const sf::RenderWindow& window, const sf::Event& event) {
//     switch (event.type) {
//     case sf::Event::KeyPressed:
//         pressedKeys.insert(event.key.scancode);
//         break;
//     case sf::Event::KeyReleased:
//         pressedKeys.erase(event.key.scancode);
//         break;
//     case sf::Event::MouseButtonPressed:
//         if (!pressedMouseButtons.contains(event.mouseButton.button)) {
//             justPressedMouseButtons.insert(event.mouseButton.button);

//             auto now = std::chrono::steady_clock::now();
//             if (lastClickTime.contains(event.mouseButton.button)) {
//                 auto duration = std::chrono::duration<float>(now - lastClickTime[event.mouseButton.button]).count();
//                 if (duration <= doubleClickThresholdSec) {
//                     // Handle double click, could set a flag or emit event (future proof)
//                 }
//             }
//             lastClickTime[event.mouseButton.button] = now;
//         }
//         pressedMouseButtons.insert(event.mouseButton.button);
//         break;
//     case sf::Event::MouseButtonReleased:
//         pressedMouseButtons.erase(event.mouseButton.button);
//         break;
//     default:
//         break;
//     }
// }

// void InputManager::postUpdate(const sf::RenderWindow& window) {
//     mouseScreenPosition = sf::Mouse::getPosition(window);
//     justPressedMouseButtons.clear();
// }

// sf::Vector2i InputManager::getMousePosition() const {
//     return mouseScreenPosition;
// }

// sf::Vector2f InputManager::getMousePositionWorld(const sf::RenderWindow& window) const {
//     return window.mapPixelToCoords(mouseScreenPosition);
// }

// bool InputManager::isKeyPressed(sf::Keyboard::Scancode key) const {
//     return pressedKeys.contains(key);
// }

// bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
//     return pressedMouseButtons.contains(button);
// }

// bool InputManager::isMouseJustPressed(sf::Mouse::Button button) const {
//     return justPressedMouseButtons.contains(button);
// }

// bool InputManager::isDoubleClick(sf::Mouse::Button button) const {
//     // Could be implemented explicitly with extra state if needed.
//     return false;
// }
