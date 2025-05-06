#include "ReceptionIcon.hpp"
#include "../Common/Common.hpp"

ReceptionIcon::ReceptionIcon(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos, std::string state)
{

    //calculate the iconPosition
    iconPosition=getPointOnLine(senderPos,receiverPos);
    icon.setPosition(iconPosition);

    //load the texture
    loadTextures("assets/Reception/interference.png", "assets/Reception/notListening.png", "assets/Reception/allGood.png");
    if(state=="interference"){
        icon.setTexture(interferenceTexture);
    }
    else if(state=="notListening"){
        icon.setTexture(notListeningTexture);
    }
    else if(state=="received"){
        icon.setTexture(receivedTexture);
    }
    else{
        throw std::runtime_error("Invalid reception state");
    }
        icon.setScale(radiusIcon / icon.getLocalBounds().width, radiusIcon / icon.getLocalBounds().height);

        // Center the icon on the calculated position
        sf::FloatRect bounds = icon.getLocalBounds();
        icon.setOrigin(bounds.width / 2, bounds.height / 2);
}

void ReceptionIcon::draw(sf::RenderWindow &window) const
{
if(!isFinished()){
    window.draw(icon);

}
}

bool ReceptionIcon::isFinished() const
{       
     if (receptionClock.getElapsedTime().asSeconds() >= receptionDuration) {

        return true;
        }
    return false;
}

bool ReceptionIcon::loadTextures(const std::string& interferencePath, const std::string& notListeningPath, const std::string& receivedPath) {
    if (!interferenceTexture.loadFromFile(interferencePath) ||
        !notListeningTexture.loadFromFile(notListeningPath) ||
        !receivedTexture.loadFromFile(receivedPath)) {
        throw std::runtime_error("Failed to load icon texture for receptionMsg");
        return false; // Return false if any texture fails to load
    }
    return true;
}

// Function to calculate the position
sf::Vector2f ReceptionIcon::getPointOnLine(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos) {
    // Compute the direction vector
    sf::Vector2f direction = senderPos - receiverPos;
    
    // Calculate the length of the direction vector
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    // Normalize the direction vector (avoid division by zero)
    if (length != 0) {
        direction /= length;
    }

    // Scale the normalized direction vector by the radius
    sf::Vector2f result = receiverPos + direction * (float)radiusIcon;

    return result;
}