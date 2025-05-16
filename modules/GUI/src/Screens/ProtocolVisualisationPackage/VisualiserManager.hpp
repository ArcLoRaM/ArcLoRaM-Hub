#ifndef VISUALISERMANAGER_HPP
#define VISUALISERMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include "../../Visualisation/Device/Device.hpp"
#include "../../Ui/Button/Button.hpp"
#include "../../Visualisation/Animations/Arrow/Arrow.hpp"
#include "../../Visualisation/Animations/BroadcastAnimation/BroadcastAnimation.hpp"

#include "../../Visualisation/Animations/PacketDrop/PacketDrop.hpp"
#include "../../Visualisation/Animations/ReceptionIcon/ReceptionIcon.hpp"
#include "ProtocolVisualisationState.hpp"
#include "../../Shared/InputManager/InputManager.hpp"

class VisualiserManager {
private:
    sf::Font* font; // Font for rendering text
    std::optional< sf::Text> text; // SFML text object
    std::optional< sf::Text> communicationMode;
    std::optional< sf::Text> tickNb;

    std::optional< sf::Text> nbRetransmission;
    sf::String nbRetransmissionString;
    std::optional< sf::Text> energyExpenditure;
    sf::String energyExpenditureString;


    //Routing
    std::unordered_set<int> devicesId; // Stores unique devices ID
    std::unordered_map<int, std::unordered_set<int>> routings; // Oriented Graph ! 2->4 is not equl to 4->2
    bool routingDisplayEnabled = false; // Flag to control routing display
    void drawRootings(sf::RenderWindow& window);

    std::vector<std::unique_ptr<Button>> buttons; // List of buttons
    mutable std::mutex buttonsMutex;

    std::vector<std::unique_ptr<Device>> devices; 
    mutable std::mutex devicesMutex;  // Protects 'devices' for all access (including reads and writes)

    std::vector<std::unique_ptr<Arrow>> arrows; // List of arrows
    mutable std::mutex arrowsMutex; // Protects 'arrows' for all access (including reads and writes)
    std::vector<std::unique_ptr<PacketDrop>> dropAnimations; 
    mutable std::mutex dropAnimationsMutex; 
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations; 
    mutable std::mutex broadcastAnimationsMutex; 
    std::vector<std::unique_ptr<ReceptionIcon>> receptionIcons;
    mutable std::mutex receptionIconsMutex; 

public:
    VisualiserManager();
    void update(InputManager & inputManager);
    void draw(sf::RenderWindow& window, sf::View& networkView,const ProtocolVisualisationState& state);

    void updateDevicesState(int nodeId, std::string state);

    void addButton(std::unique_ptr<Button> button);
    void addDevice(std::unique_ptr<Device> device);
    void addArrow(std::unique_ptr<Arrow> arrow);
    void addReceptionIcon(std::unique_ptr<ReceptionIcon> receptionIcon);
    void addDropAnimation(std::unique_ptr<PacketDrop> dropAnimation);
    void changeArrowState(int senderId, int receiverId, std::string state);
    std::pair<sf::Vector2f, bool> findDeviceCoordinates(int nodeId);

      // Animations
    void startBroadcast(const sf::Vector2f& startPosition, float duration);

    //Routing:
    void addDeviceId(int id);
    void addRouting(int id1, int id2);
    void removeRouting(int id1, int id2);

};


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



#endif // VISUALISERMANAGER_HPP
