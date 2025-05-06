#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <mutex>
#include <cmath>
#include "../Button/Button.hpp"
#include "../VisualiserManager/VisualiserManager.hpp"

//shared data
#pragma once
//TODO: put const in const values
//shared data
extern std::vector<std::string> logMessages;  // Shared container for received strings
extern std::mutex logMutex;                  // Mutex to protect the container
extern bool isRunning;                // Flag to signal threads to stop

extern std::string COMMUNICATION_MODE;
extern int TICK_NB; 

extern float DISTANCE_THRESHOLD;
extern float broadcastDuration;

extern float arrowDuration;
extern float receptionDuration;

extern std::mutex deviceMutex;

extern double radiusIcon;

//so everything fits in the screen available to the eyes of the user
extern int verticalOffset;
extern int horizontalOffset;
extern float distanceDivider;
//TODO: make every UI elements Proportionnal to windowWidth and Heigt, to allow resizing. Right now, it's a fixed size

//General DATA:
extern  const int windowWidth;
extern  const int windowHeight;
extern int NBRETRANSMISSION;
//Listen-> 1 energy unit. Transmit -> 20 energy unit
extern int ENERGYEXP;

//Buttons:
extern const std::string on;
extern const std::string off;

extern const float x1coor;
extern const float y1coor;
extern const float width1;
extern const float height1;
extern const sf::Color color1;
extern std::string state1;
extern const std::string iconPath1;

extern const float x2coor;
extern const float y2coor;
extern const float width2;
extern const float height2;
extern const sf::Color color2;
extern std::string state2;
extern const std::string iconPath2;

extern const float x3coor;
extern const float y3coor;
extern const float width3;
extern const float height3;
extern const sf::Color color3;
extern std::string state3;
extern const std::string iconPath3;

extern const float x4coor;
extern const float y4coor;
extern const float width4;
extern const float height4;
extern const sf::Color color4;
extern std::string state4;
extern const std::string iconPath4;



//Animations----------------------------------------------
inline // Helper function to convert std::pair<int, int> to sf::Vector2f
sf::Vector2f pairToVector2f(const std::pair<int, int>& pair) {
    return sf::Vector2f(static_cast<float>(pair.first), static_cast<float>(pair.second));
}

extern const sf::Color dataArrowColor;
extern const sf::Color ackArrowColor;
 
// Function to create an arrowhead shape
inline sf::ConvexShape createArrowhead(sf::Vector2f position, sf::Vector2f direction, float headLength, float headWidth) {
    // Normalize the direction
    float magnitude = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    direction /= magnitude;

    // Perpendicular vector for arrowhead width
    sf::Vector2f perp(-direction.y, direction.x);

    // Arrowhead points
    sf::Vector2f tip = position;
    sf::Vector2f base1 = position - direction * headLength + perp * (headWidth / 2.0f);
    sf::Vector2f base2 = position - direction * headLength - perp * (headWidth / 2.0f);

    sf::ConvexShape arrowhead;
    arrowhead.setPointCount(3);
    arrowhead.setPoint(0, tip);
    arrowhead.setPoint(1, base1);
    arrowhead.setPoint(2, base2);
    arrowhead.setFillColor(sf::Color::Yellow);

    return arrowhead;
}

// Function to draw a shaft and multiple arrowheads along the shaft
inline void drawArrowWithHeads(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end, float headSpacing) {
    // Calculate the direction vector and length of the arrow
    sf::Vector2f direction = end - start;
    float totalLength = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Normalize the direction vector
    direction /= totalLength;

    // Arrow properties
    float shaftThickness = 5.0f; // Thickness of the shaft
    float headLength = 20.0f;    // Length of each arrowhead
    float headWidth = 15.0f;     // Width of each arrowhead

    // Calculate the perpendicular vector for the shaft's thickness
    sf::Vector2f perp(-direction.y, direction.x);

    // Points for the shaft
    sf::Vector2f shaftStart1 = start + perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftStart2 = start - perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftEnd1 = end - perp * (shaftThickness / 2.0f);
    sf::Vector2f shaftEnd2 = end + perp * (shaftThickness / 2.0f);

    // Draw the shaft
    sf::ConvexShape shaft;
    shaft.setPointCount(4);
    shaft.setPoint(0, shaftStart1);
    shaft.setPoint(1, shaftStart2);
    shaft.setPoint(2, shaftEnd2);
    shaft.setPoint(3, shaftEnd1);
    shaft.setFillColor(sf::Color::Yellow);
    window.draw(shaft);

    // Place arrowheads along the shaft
    float currentLength = headSpacing; // Start after the first headSpacing
    while (currentLength < totalLength) {
        // Calculate the position for the current arrowhead
        sf::Vector2f headPosition = start + direction * currentLength;

        // Create and draw the arrowhead
        sf::ConvexShape arrowhead = createArrowhead(headPosition, direction, headLength, headWidth);
        window.draw(arrowhead);

        // Move to the next position
        currentLength += headSpacing;
    }
}


