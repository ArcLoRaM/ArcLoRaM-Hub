#include "Arrow.hpp"
#include <cmath> // For trigonometric functions
#include "../Common/Common.hpp"

// Constructor
Arrow::Arrow(const sf::Vector2f& start, const sf::Vector2f& end,  int senderId, int receiverId, sf::Color color)
    : start(start), end(end), progress(0.0f), duration(arrowDuration), SenderId(senderId), ReceiverId(receiverId), receptionDuration(receptionDuration), receptionComplete(false), color(color) {
    // Initialize line (shaft of the arrow)
    line.setSize(sf::Vector2f(0, 7)); // Initially, the line length is 0, thickness = 7
    line.setFillColor(sf::Color::Red);

    // Initialize arrowhead (a triangle)
    arrowhead.setPointCount(3);
    arrowhead.setFillColor(sf::Color::Red);

    loadTextures("assets/Reception/interference.png", "assets/Reception/notListening.png", "assets/Reception/allGood.png");
    receptionState = "received"; // Default reception state
    
}

// Load textures for the reception state icons
bool Arrow::loadTextures(const std::string& interferencePath, const std::string& notListeningPath, const std::string& receivedPath) {
    if (!interferenceTexture.loadFromFile(interferencePath) ||
        !notListeningTexture.loadFromFile(notListeningPath) ||
        !receivedTexture.loadFromFile(receivedPath)) {
        throw std::runtime_error("Failed to load icon texture for receptionMsg");
        return false; // Return false if any texture fails to load
    }
    return true;
}

// Calculate the angle between two points
float Arrow::calculateAngle(const sf::Vector2f& p1, const sf::Vector2f& p2) const {
    return std::atan2(p2.y - p1.y, p2.x - p1.x) * 180.0f / 3.14159265f;
}

// Update the arrow animation
void Arrow::update() {
    // Update progress based on elapsed time
    float elapsed = clock.getElapsedTime().asSeconds();
    progress = elapsed / arrowDuration;

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
    line.setRotation(calculateAngle(start, currentPosition));

    // Update arrowhead position and rotation
    float angle = calculateAngle(line.getPosition(), currentPosition);
    arrowhead.setPosition(currentPosition);
    arrowhead.setRotation(angle);

    // Define the triangle points for the arrowhead
    float arrowSize = 35.0f; // Size of the arrowhead
    arrowhead.setPoint(0, sf::Vector2f(0, 0));
    arrowhead.setPoint(1, sf::Vector2f(-arrowSize, arrowSize / 2));
    arrowhead.setPoint(2, sf::Vector2f(-arrowSize, -arrowSize / 2));

    // If the animation is finished, start the reception clock
    if (isFinished() && !receptionComplete) {
        icon.setPosition(currentPosition);

        // Set the icon texture based on receptionState
        if (receptionState == "interference") {
            icon.setTexture(interferenceTexture);
        } else if (receptionState == "notListening") {
            icon.setTexture(notListeningTexture);
        } else if (receptionState == "received") {
            icon.setTexture(receivedTexture);
        }
icon.setScale(arrowSize / icon.getLocalBounds().width, arrowSize / icon.getLocalBounds().height);

        // Center the icon on the arrow's tip
        sf::FloatRect bounds = icon.getLocalBounds();
        icon.setOrigin(bounds.width / 2, bounds.height / 2);

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
        window.draw(icon);
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
