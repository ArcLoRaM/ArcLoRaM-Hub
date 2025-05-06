#ifndef BROADCASTANIMATION_HPP
#define BROADCASTANIMATION_HPP

#include <SFML/Graphics.hpp>

class BroadcastAnimation {
public:
    // Constructor
    BroadcastAnimation(const sf::Vector2f& startPosition, float duration);

    // Update the animation state (time-based, no deltaTime required)
    void update();

    // Draw the animation
    void draw(sf::RenderWindow& window) const;

    // Check if the animation is finished
    bool isFinished() const;

    // Check if the reception icon duration has completed
    bool isReceptionFinished() const;

    // Set the duration for the reception icon display
    void setReceptionDuration(float duration);

private:
    sf::CircleShape circle;     // Expanding circle shape
    sf::Vector2f startPosition; // Starting position of the circle
    float duration;             // Total duration of the animation (seconds)
    sf::Clock clock;            // Clock to track elapsed time

    //Reception:
    std::vector<sf::Sprite>  receptionIcons;  // Sprite for the reception state icon
    sf::Texture interferenceTexture;
    sf::Texture notListeningTexture;
    sf::Texture receivedTexture;
    
    sf::Clock receptionClock;   // Clock for the reception icon duration, common for every reception
    float receptionDuration;   // Duration to display the reception icon

    bool receptionComplete;    // Flag to check if the reception is complete
    
    // Load textures for state icons
    bool loadTextures(const std::string& interferencePath, const std::string& notListeningPath, const std::string& receivedPath);
};

#endif // BROADCASTANIMATION_HPP
