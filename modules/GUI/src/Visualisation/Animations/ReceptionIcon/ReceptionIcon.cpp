#include "ReceptionIcon.hpp"
#include "../../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../../Shared/Config.hpp"
#include <cmath>
ReceptionIcon::ReceptionIcon(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos, std::string state)
{

    //calculate the iconPosition
    iconPosition=getPointOnLine(senderPos,receiverPos);

    //load the texture
    interferenceTexture = &ResourceManager::getInstance().getTexture("Reception_Interference");
    notListeningTexture = &ResourceManager::getInstance().getTexture("Reception_NotListening");
    receivedTexture = &ResourceManager::getInstance().getTexture("Reception_AllGood");
   
    if(state=="interference"){
        icon.emplace(*interferenceTexture);
    }
    else if(state=="notListening"){
        icon.emplace(*notListeningTexture);
    }
    else if(state=="received"){
        icon.emplace(*receivedTexture);
    }
    else{
        throw std::runtime_error("Invalid reception state");
    }

        icon->setScale(sf::Vector2f(config::radiusIcon/icon->getLocalBounds().size.x,config::radiusIcon/icon->getLocalBounds().size.y));
        icon->setPosition(iconPosition);

        // Center the icon on the calculated position
        sf::FloatRect bounds = icon->getLocalBounds();
        icon->setOrigin(sf::Vector2f( bounds.size.x / 2, bounds.size.y / 2));
}

void ReceptionIcon::draw(sf::RenderWindow &window) const
{
if(!isFinished()){
    window.draw(*icon);

}
}

bool ReceptionIcon::isFinished() const
{       
     if (receptionClock.getElapsedTime().asSeconds() >= config::receptionDuration) {

        return true;
        }
    return false;
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
    sf::Vector2f result = receiverPos + direction * (float)config::radiusIcon;

    return result;
}