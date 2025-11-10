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
    sf::Color color;  
    // Helper to calculate angle for the arrowhead
    float calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2) const;
    
public:

    int SenderId;   
    int ReceiverId;
    std::string receptionState; 

    // Constructor
    Arrow(const sf::Vector2f& start, const sf::Vector2f& end,  int senderId, int receiverId, sf::Color color = sf::Color::Red);


    // Draw the arrow on the given window
    void draw(tgui::CanvasSFML::Ptr canvas);

};

#endif // ARROW_HPP
