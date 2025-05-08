#ifndef VISUALISERMANAGER_HPP
#define VISUALISERMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>
#include "../Device/Device.hpp"
#include "../Button/Button.hpp"
#include "../Common/Common.hpp"
#include "../Arrow/Arrow.hpp"
#include "../BroadcastAnimation/BroadcastAnimation.hpp"
#include <unordered_map>
#include <unordered_set>
#include "../ReceptionIcon/ReceptionIcon.hpp"
#include "../PacketDrop/PacketDrop.hpp"



//everything related to the protocol itself is done here
//the rest: button, logs, is done in the display thread
class VisualiserManager {
private:
    sf::Font font; // Font for rendering text
    std::optional< sf::Text> text; // SFML text object
    std::optional< sf::Text> communicationMode;
    std::optional< sf::Text> tickNb;

    std::optional< sf::Text> nbRetransmission;
    sf::String nbRetransmissionString;
    std::optional< sf::Text> energyExpenditure;
    sf::String energyExpenditureString;

    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations; // List of active broadcast animations
    std::vector<std::unique_ptr<ReceptionIcon>> receptionIcons; // List of active reception icons  

    //Routing
    std::unordered_set<int> devicesId; // Stores unique devices ID
    std::unordered_map<int, std::unordered_set<int>> routings; // Oriented Graph ! 2->4 is not equl to 4->2
    void drawRootings(sf::RenderWindow& window);

public:
    VisualiserManager();
    void update();
    void draw(sf::RenderWindow& window);
    std::vector<std::unique_ptr<Button>> buttons; // List of buttons
    std::vector<std::unique_ptr<Device>> devices; // List of devices (if used elsewhere)
    std::vector<std::unique_ptr<Arrow>> arrows; // List of arrows
    std::vector<std::unique_ptr<PacketDrop>> dropAnimations; // List of packet drop animations

    void addButton(std::unique_ptr<Button> button);
    void addDevice(std::unique_ptr<Device> device);
    void addArrow(std::unique_ptr<Arrow> arrow);
    void addReceptionIcon(std::unique_ptr<ReceptionIcon> receptionIcon);
    void addDropAnimation(std::unique_ptr<PacketDrop> dropAnimation);


    void changeArrowState(int senderId, int receiverId, std::string state);


      // Animations
    void startBroadcast(const sf::Vector2f& startPosition, float duration);

    //Routing:
    void addDeviceId(int id);
    void addRouting(int id1, int id2);
    void removeRouting(int id1, int id2);

    


};

#endif // VISUALISERMANAGER_HPP
