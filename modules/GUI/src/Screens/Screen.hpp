#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "../Shared/InputManager/InputManager.hpp"
#include <functional>

using ScreenAction = std::function<void()>;
class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(InputManager& input) = 0;
    virtual void update(float deltaTime,InputManager &input) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    //make a list of key-value pairs to store the lambdas:
    // using ScreenChangeCallback = std::function<void(std::unique_ptr<Screen>)>;

};
