#include "Arrow.hpp"
#include <cmath> // For trigonometric functions
#include "../../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../../Shared/Config.hpp"
// Constructor
Arrow::Arrow(const sf::Vector2f& start, const sf::Vector2f& end,  int senderId, int receiverId, sf::Color color)
    : start(start), end(end), progress(0.0f), duration(config::arrowDuration), SenderId(senderId), ReceiverId(receiverId), receptionDuration(receptionDuration), receptionComplete(false), color(color) {
    // Initialize line (shaft of the arrow)

    line.setSize(sf::Vector2f(0, 7)); // Initially, the line length is 0, thickness = 7
    line.setFillColor(color);

    // Initialize arrowhead (a triangle)
    arrowhead.setPointCount(3);
    arrowhead.setFillColor(color);

    receptionState = "received"; // Default reception state
    interferenceTexture = &ResourceManager::getInstance().getTexture("Reception_Interference");
    notListeningTexture = &ResourceManager::getInstance().getTexture("Reception_NotListening");
    receivedTexture = &ResourceManager::getInstance().getTexture("Reception_AllGood");

    icon = std::make_optional<sf::Sprite>(*receivedTexture);
    
}


// Calculate the angle between two points
float Arrow::calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2) const {
    return std::atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159265f;
}

// Update the arrow animation
void Arrow::update() {
    // Update progress based on elapsed time
    float elapsed = clock.getElapsedTime().asSeconds();
    progress = elapsed / config::arrowDuration;

    if (progress > 0.9f) { //we shorten a bit the arrow for clarity TODO: make the progress stopped at a fixed distance from the receiver, not relative because it might be weird when there is a lot of distance between two nodes.

        progress = 0.9f; // Clamp progress
    }

    // Interpolate current position
    sf::Vector2f currentPosition = start + progress * (end - start);

    // Update line
    sf::Vector2f direction = currentPosition - start;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y) * 0.95; // 0.95 prevents overstepping
    line.setSize(sf::Vector2f(length, 7)); // Adjust the line's length and thickness
    line.setPosition(start);
    line.setRotation(sf::degrees(calculateAngle(start, currentPosition)));

    // Update arrowhead position and rotation
    float angle = calculateAngle(line.getPosition(), currentPosition);
    arrowhead.setPosition(currentPosition);
    arrowhead.setRotation(sf::degrees(angle));

    // Define the triangle points for the arrowhead
    float arrowSize = 35.0f; // Size of the arrowhead
    arrowhead.setPoint(0, sf::Vector2f(0, 0));
    arrowhead.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2));
    arrowhead.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize / 2));

    // If the animation is finished, start the reception clock
    if (isFinished() && !receptionComplete) {
        icon->setPosition(currentPosition);

        // Set the icon texture based on receptionState
        if (receptionState == "interference") {
            icon->setTexture(*interferenceTexture);
        } else if (receptionState == "notListening") {
            icon->setTexture(*notListeningTexture);
        } else if (receptionState == "received") {
            icon->setTexture(*receivedTexture);
        }
       sf::Vector2f factors=sf::Vector2f(arrowSize / icon->getLocalBounds().size.x, arrowSize / icon->getLocalBounds().size.y);
        icon->setScale(factors);

        // Center the icon on the arrow's tip
        sf::FloatRect bounds = icon->getLocalBounds();
        sf::Vector2f newneworigin(bounds.size.x/ 2, bounds.size.y / 2);
        icon->setOrigin(newneworigin);

        // Start the reception timer
        if (receptionClock.getElapsedTime().asSeconds() >= receptionDuration) {
            receptionComplete = true;
        }
    }
}

// Draw the arrow and the icon (if applicable)
void Arrow::draw(sf::RenderWindow& window) {
    window.draw(line);
    window.draw(arrowhead);

    if (isFinished() && !receptionComplete) {
        window.draw(*icon);
    }
}

// Check if the animation is finished
bool Arrow::isFinished() const {
    return progress >= 0.9f;
}

// Check if the reception duration has finished
bool Arrow::isReceptionFinished() const {
    return receptionComplete;
}

// Restart the animation
void Arrow::restart() {
    progress = 0.0f;
    clock.restart();
    receptionClock.restart();
    receptionComplete = false;
}

// Set the duration for the reception icon display
void Arrow::setReceptionDuration(float duration) {
    receptionDuration = duration;
}
