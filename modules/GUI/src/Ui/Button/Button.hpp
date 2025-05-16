#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <stdexcept>
#include <functional>
#include "../../Shared/InputManager/InputManager.hpp"

class Button {
private:
    sf::RectangleShape shape;
    std::optional<sf::Sprite> icon;
    sf::Texture* iconTexture = nullptr;

    bool isHovered = false;

    std::function<void()> onClickAction; // callback for click action

public:
    Button(float x, float y, float width, float height,
           sf::Color color, const std::string& resourceKey);

    void draw(sf::RenderWindow& window);
    void update(const InputManager& input);

    void setOnClick(std::function<void()> callback);
};


#endif // BUTTON_HPP
