#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(const std::optional<sf::Event> event) = 0;
    virtual void update(float deltaTime) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
};
