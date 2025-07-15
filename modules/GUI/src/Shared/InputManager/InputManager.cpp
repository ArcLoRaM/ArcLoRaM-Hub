#include "InputManager.hpp"
#include <iostream>

void InputManager::handleEvent(const std::optional<sf::Event> &event)
{
    if (!event)
        return;

    if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
    {
        pressedKeys.insert(keyPressed->scancode);
    }
    else if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>())
    {
        pressedKeys.erase(keyReleased->scancode);
    }
    else if (const auto *mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
    {
        // if (!pressedMouseButtons.contains(mousePressed->button)) {
        //     justPressedMouseButtons.insert(mousePressed->button);
        //     std::cout << "Mouse button pressed: " << static_cast<int>(mousePressed->button) << std::endl;
        // }

        auto now = std::chrono::steady_clock::now();

        if (!pressedMouseButtons.contains(mousePressed->button))
        {
            justPressedMouseButtons.insert(mousePressed->button);

            // Double-click detection
            auto it = lastClickTime.find(mousePressed->button);
            if (it != lastClickTime.end())
            {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second);
                if (elapsed < doubleClickThreshold)
                {
                    doubleClickedMouseButtons.insert(mousePressed->button);
                }
            }
            lastClickTime[mousePressed->button] = now;

            std::cout << "Mouse button pressed: " << static_cast<int>(mousePressed->button) << std::endl;
        }
        pressedMouseButtons.insert(mousePressed->button);
    }
    else if (const auto *mouseReleased = event->getIf<sf::Event::MouseButtonReleased>())
    {
        pressedMouseButtons.erase(mouseReleased->button);
    }
    else if (const auto *wheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())

    {
        if (wheelScrolled->wheel == sf::Mouse::Wheel::Vertical)
        {
            accumulatedMouseWheelDelta += wheelScrolled->delta;
        }
    }

    else if (const auto *textEntered = event->getIf<sf::Event::TextEntered>())
    {
        char32_t unicode = textEntered->unicode;
        // Accept ASCII range + control characters like backspace and enter
        if (unicode < 128)
        {
            textBuffer += unicode;
        }
        else if (unicode == 8)
        { // backspace
            if (!textBuffer.empty())
                textBuffer.pop_back();
        }
    }
}

void InputManager::postUpdate(const sf::RenderWindow &window)
{

    // the mouse position in the given window
    mouseScreenPosition = sf::Mouse::getPosition(window);

    // use the current view as reference
    mouseWorldPosition = window.mapPixelToCoords(mouseScreenPosition);

    // UI position use default view explicitly
    mouseUIScreenPosition = window.mapPixelToCoords(mouseScreenPosition, window.getDefaultView());

    justPressedMouseButtons.clear(); // Reset per-frame
    accumulatedMouseWheelDelta = 0.f;

    doubleClickedMouseButtons.clear();
}


//honestly This is super messy
sf::Vector2i InputManager::getMouseScreenPosition() const
{
    return mouseScreenPosition;
}

sf::Vector2f InputManager::getMouseWorldPosition() const
{
    return mouseWorldPosition;
}
sf::Vector2f InputManager::getMouseUIScreenPosition() const
{
    return mouseUIScreenPosition;
}
bool InputManager::isKeyPressed(sf::Keyboard::Scancode key) const
{
    return pressedKeys.contains(key);
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const
{
    return pressedMouseButtons.contains(button);
}

bool InputManager::isMouseJustPressed() const
{
    return !justPressedMouseButtons.empty();
}

float InputManager::getMouseWheelDelta() const
{
    return accumulatedMouseWheelDelta;
}

bool InputManager::isRightMouseJustPressed() const
{
    return justPressedMouseButtons.contains(sf::Mouse::Button::Right);
}

bool InputManager::isLeftMouseJustPressed() const
{
    return justPressedMouseButtons.contains(sf::Mouse::Button::Left);
}

const std::u32string &InputManager::getTextBuffer() const
{
    return textBuffer;
}

void InputManager::clearTextBuffer()
{
    textBuffer.clear();
}