#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Shared/Config.hpp"

enum class DeviceState {
    Sleep,
    Listen,
    Transmit,
    Communicate
};

enum class DeviceClass {
    C1,
    C2,
    C3
};


#pragma once

struct DeviceMetrics {
public:
    //something to have in mind: the energy expenditure for one slot listening can actually be very small if we trust our synchronization enough to consider there is no one transmitting 
    
    // Energy slot counters
    void incrementListeningData()         { ++slotListeningData; }
    void incrementTransmittingData()      { --slotListeningData; ++slotTransmittingData; }
    void incrementListeningAck()          { ++slotListeningAck; }
    void incrementTransmittingAck()       { --slotListeningAck; ++slotTransmittingAck; }

    // Transmission stats
    void incrementRetransmission()        { ++nbRetransmissions; }
    void incrementPacketSent()            { ++nbPacketsSent; }

    void reset() {
        slotListeningData = 0;
        slotTransmittingData = 0;
        slotListeningAck = 0;
        slotTransmittingAck = 0;
        
        nbRetransmissions = 0;
        nbPacketsSent = 0;
    }

    // Accessors for CSV export
    int getListeningDataSlots()     const { return slotListeningData; }
    int getTransmittingDataSlots()  const { return slotTransmittingData; }
    int getListeningAckSlots()      const { return slotListeningAck; }
    int getTransmittingAckSlots()   const { return slotTransmittingAck; }

    int getRetransmissions()        const { return nbRetransmissions; }
    int getPacketsSent()            const { return nbPacketsSent; }

    // Derived metric (if needed)
    double getPacketDeliveryRatio() const {
        return nbPacketsSent > 0
            ? static_cast<double>(nbPacketsSent - nbRetransmissions) / nbPacketsSent
            : 0.0;
    }

    // Simplified energy estimate (can be refined)
    int getTotalEnergySlots() const {
        return slotListeningData + slotTransmittingData +
               slotListeningAck + slotTransmittingAck;
    }

private:
    // Energy expenditure slots
    int slotListeningData = 0;
    int slotTransmittingData = 0;
    int slotListeningAck = 0;
    int slotTransmittingAck = 0;

    // Packet transmission metrics
    int nbRetransmissions = 0;
    int nbPacketsSent = 0;
};

class Device {
private:
    sf::CircleShape shape;
    sf::Texture* iconTexture = nullptr;

    std::optional<sf::Text> infoTextId;
    std::optional<sf::Text> infoTextBattery;
    std::optional<sf::Text> infoTextCoordinates;
    sf::RectangleShape infoWindow;

    std::string textId;
    std::string textBattery;
    std::string textCoordinates;


    sf::Font* font = nullptr;

    bool displayInfoWindow = false;
    bool isHovered = false;
    sf::Vector2f centeredPosition;


    //Do a proper enum for the device Info if it grows larger: Todo.
    int nodeId = 0;
    DeviceClass classNode ;
    double batteryLevel = 0;
    DeviceState state = DeviceState::Sleep;
    int hopCount = 0;
    std::string getTextureKey(DeviceClass cls, DeviceState state);



    

public:

    Device(int nodeId, DeviceClass classNode, sf::Vector2f centeredPosition,int hopCount, double batteryLevel = 0);

    void updateCoordinatesString();

    void draw(sf::RenderWindow& window);
    void update(const InputManager& input);

    void setState(DeviceState newState);

    //return the position of the Shape (top left corner)
    sf::Vector2f getPosition() const {
        return shape.getPosition();
    }

    //return the centered position of the Shape (you want to use this one most of the time)
    sf::Vector2f getCenteredPosition() const {
        return centeredPosition;
    }

    int getNodeId() const {
        return nodeId;
    }
    int getHopCount() const {
        return hopCount;
    }

    void changePosition(const sf::Vector2f& newCenteredPos) {
        centeredPosition = newCenteredPos;
        shape.setPosition(centeredPosition - sf::Vector2f(config::radiusIcon, config::radiusIcon));
        updateCoordinatesString();
    }

    sf::Vector2f getSize() const {
        return shape.getGlobalBounds().size;
    }
    bool getIsHovered() const {
        return isHovered;
    }
    DeviceClass getClass() const { return classNode; }


    DeviceMetrics metrics;

};


