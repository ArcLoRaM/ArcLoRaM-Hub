#pragma once

#include <SFML/Graphics.hpp>
#include <optional>
#include <functional>
#include "../../Shared/InputManager/InputManager.hpp"

class Button {
    sf::RectangleShape shape;
    std::optional<sf::Sprite> icon;
    std::optional<sf::Text> text;
    const sf::Texture* iconTexture = nullptr;
    const sf::Font* font = nullptr;
    std::function<void()> onClickAction;
    bool isHovered = false;

public:
    Button(float x, float y, float width, float height, sf::Color color);//Empty 
    Button(float x, float y, float width, float height, sf::Color color, const std::string& resourceKey);//Icon
    Button(float x, float y, float width, float height, sf::Color color, const std::string& labelText, const std::string& fontKey);//text

    void draw(sf::RenderWindow& window);
    void update(const InputManager& input);
    void setOnClick(std::function<void()> callback);
};
