#pragma once
#include <SFML/Graphics.hpp>
#include <cmath>

//For the routing Arrow
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
inline void drawArrowWithHeads(tgui::CanvasSFML::Ptr canvas, sf::Vector2f start, sf::Vector2f end, float headSpacing) {
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
    canvas->draw(shaft);

    // Place arrowheads along the shaft
    float currentLength = headSpacing; // Start after the first headSpacing
    while (currentLength < totalLength) {
        // Calculate the position for the current arrowhead
        sf::Vector2f headPosition = start + direction * currentLength;

        // Create and draw the arrowhead
        sf::ConvexShape arrowhead = createArrowhead(headPosition, direction, headLength, headWidth);
        canvas->draw(arrowhead);

        // Move to the next position
        currentLength += headSpacing;
    }
}

