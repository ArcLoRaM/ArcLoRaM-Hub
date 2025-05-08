#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <stdexcept>

class Button {
private:
    sf::RectangleShape shape;
    std::string& state; // Reference to the global state this button controls
    std::optional< sf::Sprite >icon;    // Icon inside the button
    sf::Texture iconTexture; // Texture for the icon
    std::string onState;
    std::string offState;

public:
    // Constructor
    Button(float x, float y, float width, float height,
        sf::Color color, std::string& stateRef,
        const std::string& on, const std::string& off, const std::string& iconPath);

    // Draw the button
    void draw(sf::RenderWindow& window);

    // Handle events like mouse clicks
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);

    // Get the current state of the button
    std::string getState() const;
};

#endif // BUTTON_HPP
