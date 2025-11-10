#ifndef DYNAMICBROADCAST_HPP
#define DYNAMICBROADCAST_HPP

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
class DynamicBroadcast {
public:
    // Constructor
    DynamicBroadcast(const sf::Vector2f& startPosition, float duration);

    // Update the animation state (time-based, no deltaTime required)
    void update();

    // Draw the animation
    void draw(tgui::CanvasSFML::Ptr canvas) const;

    // Check if the animation is finished
    bool isFinished() const;


private:
    sf::CircleShape circle;     // Expanding circle shape
    sf::Vector2f startPosition; // Starting position of the circle
    float duration;             // Total duration of the animation (seconds)
    sf::Clock clock;            // Clock to track elapsed time

    sf::Clock receptionClock;   // Clock for the reception icon duration, common for every reception
    float receptionDuration;   // Duration to display the reception icon

    bool receptionComplete;    // Flag to check if the reception is complete
    
};

#endif // BROADCASTANIMATION_HPP