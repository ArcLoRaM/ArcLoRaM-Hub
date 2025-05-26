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

    std::optional<sf::Text> pdrText;
    std::string pdrString = "PDR: ";

    //Routing
    //to do: use the same data structure as in the TopologyEditorManager for the Id and the ptdr to the devices, all in one ! fantastic
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
    VisualiserManager(ProtocolVisualisationState& state);
    const std::vector<std::unique_ptr<Device>>& getDevices() const { return devices; }
    std::mutex& getDevicesMutex() const { return devicesMutex; }
    void update(InputManager & inputManager);
    void draw(sf::RenderWindow& window, sf::View& networkView, ProtocolVisualisationState& state);

    void updateDevicesState(int nodeId,  DeviceState state);

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


    //metrics
    void addRetransmission(int nodeId);
    void incrementPacketSent(int nodeId);

};



#endif // VISUALISERMANAGER_HPP
