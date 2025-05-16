#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "../Shared/InputManager/InputManager.hpp"

class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(InputManager& input) = 0;
    virtual void update(float deltaTime,InputManager &input) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};
