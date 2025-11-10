#include "Arrow.hpp"
#include <cmath> // For trigonometric functions
#include "../../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../../Shared/Config.hpp"
// Constructor
Arrow::Arrow(const sf::Vector2f& start, const sf::Vector2f& end,  int senderId, int receiverId, sf::Color color)
    : start(start), end(end), SenderId(senderId), ReceiverId(receiverId), color(color) {
    

    // Setup line
    sf::Vector2f direction = end - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y) * 0.95f;
    line.setSize(sf::Vector2f(length, 7)); // Initially, the line length is 0, thickness = 7
    line.setFillColor(color);
    line.setPosition(start);
    line.setRotation(sf::degrees(calculateAngle(start, end)));

    // Initialize arrowhead (a triangle)
    float arrowSize = 35.0f;
    arrowhead.setPointCount(3);
    arrowhead.setFillColor(color);
    arrowhead.setPoint(0, sf::Vector2f(0, 0));
    arrowhead.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2));
    arrowhead.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize / 2));
    arrowhead.setPosition(end);
    arrowhead.setRotation(sf::degrees(calculateAngle(start, end)));
    
}


// Calculate the angle between two points
float Arrow::calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2) const {
    return std::atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159265f;
}


// Draw the arrow and the icon (if applicable)
void Arrow::draw(tgui::CanvasSFML::Ptr canvas)  {

    canvas->draw(line);
    canvas->draw(arrowhead);
    

}





