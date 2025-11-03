#ifndef ARROW_HPP
#define ARROW_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header

class Arrow {
private:
    sf::RectangleShape line;  // Line representing the arrow's shaft
    sf::ConvexShape arrowhead;  // Triangle representing the arrowhead
    sf::Vector2f start;  // Start position
    sf::Vector2f end;    // End position
    float progress;      // Animation progress (0.0 to 1.0)
    float duration;      // Total animation time
    sf::Clock clock;     // Clock to track animation time

   

    std::optional<sf::Sprite> icon;  // Sprite for the reception state icon
    sf::Texture* interferenceTexture;
    sf::Texture* notListeningTexture;
    sf::Texture* receivedTexture;

    sf::Clock receptionClock;  // Clock for the reception icon duration
    float receptionDuration;   // Duration to display the reception icon

    bool receptionComplete;    // Flag to check if the reception is complete

    // Helper to calculate angle for the arrowhead
    float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2) const;
    sf::Color color;  // Arrow color
    
public:

    int SenderId;       // Used to display the reception state of the message
    int ReceiverId;
    std::string receptionState; 


    // Constructor
    Arrow(const sf::Vector2f& start, const sf::Vector2f& end,  int senderId, int receiverId, sf::Color color = sf::Color::Red);

    // Update the arrow animation
    void update();

    // Draw the arrow on the given window
    void draw(tgui::CanvasSFML::Ptr canvas);

    // Check if the animation is finished
    bool isFinished() const;

    // Check if the reception icon duration has completed
    bool isReceptionFinished() const;

    // Restart the animation
    void restart();

    // Set the duration for the reception icon display
    void setReceptionDuration(float duration);
};

#endif // ARROW_HPP
