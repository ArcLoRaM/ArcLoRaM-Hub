#include "ReceptionIcon.hpp"
#include "../../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../../Shared/Config.hpp"
#include <cmath>
ReceptionIcon::ReceptionIcon(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos, ReceptionState state)
{

    //calculate the iconPosition
    iconPosition=getPointOnLine(senderPos,receiverPos);

    //load the texture
    interferenceTexture = &ResourceManager::getInstance().getTexture("Reception_Interference");
    notListeningTexture = &ResourceManager::getInstance().getTexture("Reception_NotListening");
    receivedTexture = &ResourceManager::getInstance().getTexture("Reception_AllGood");
   
    if(state==ReceptionState::Interference){
        icon.emplace(*interferenceTexture);
    }
    else if(state==ReceptionState::NotListening){
        icon.emplace(*notListeningTexture);
    }
    else if(state==ReceptionState::Received){
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

void ReceptionIcon::draw(tgui::CanvasSFML::Ptr canvas) const
{
if(!isFinished()&& icon.has_value()){
    canvas->draw(*icon);

}
}

bool ReceptionIcon::isFinished() const
{       
    return receptionClock.getElapsedTime().asSeconds() >= config::receptionDuration;

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
    return receiverPos + direction * static_cast<float>(config::radiusIcon);
}