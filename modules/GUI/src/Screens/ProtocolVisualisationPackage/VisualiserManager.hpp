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
#include "../../Visualisation/Animations/Arrow/Arrow.hpp"
#include "../../Visualisation/Animations/BroadcastAnimation/BroadcastAnimation.hpp"
#include "../../Visualisation/Animations/PacketDrop/PacketDrop.hpp"
#include "../../Visualisation/Animations/ReceptionIcon/ReceptionIcon.hpp"
#include "ProtocolVisualisationState.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "CommandSender.hpp"
#include "../../Network/TcpServer/ClientSession.hpp"


class VisualiserManager {
private:
    CommandSender commandSender;

    //TODO: put rooting and Devices in the state, manager should only handle interface specific to the feature ( not like redirection between pages) and animations (transmission etc..)
    // I really think it's necessary, the sooner the better 
    
    //Routing
    std::unordered_map<int, std::unordered_set<int>> routings; // Oriented Graph ! 2->4 is not equl to 4->2
    bool routingDisplayEnabled = false; // Flag to control routing display
    void drawRootings(tgui::CanvasSFML::Ptr canvas);
    
    std::unordered_map<int, std::unique_ptr<Device>> devices;
    mutable std::mutex devicesMutex;  // Protects 'devices' for all access (including reads and writes)

    std::vector<std::unique_ptr<Arrow>> arrows; // List of arrows
    mutable std::mutex arrowsMutex; // Protects 'arrows' for all access (including reads and writes)
    std::vector<std::unique_ptr<PacketDrop>> dropAnimations; 
    mutable std::mutex dropAnimationsMutex; 
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations; 
    mutable std::mutex broadcastAnimationsMutex; 
    std::vector<std::unique_ptr<ReceptionIcon>> receptionIcons;
    mutable std::mutex receptionIconsMutex; 

    ProtocolVisualisationState& state;

  void setNetworkPanelUI(sf::View &networkView);
  void setLogsPanelUI();
  void setMetricsPanelUI();
  void setServerPanelUI();


    public:
    VisualiserManager(ProtocolVisualisationState& state, tgui::Gui &gui);
    ~VisualiserManager();

    //todo: networkView should be owned by the manager, since it's already receving the inputManager it doesnt make sense for the parent screen to handle the view
    //do the same for the editorView in TopologyEditorManager
    void setupUI(sf::View &networkView);

    const std::unordered_map<int, std::unique_ptr<Device>>& getDevices() const { return devices; }
    std::mutex& getDevicesMutex() const { return devicesMutex; }
    void update(InputManager & inputManager);
    void draw(sf::RenderWindow& window, sf::View& networkView, ProtocolVisualisationState& state);

    void updateDevicesState(int nodeId,  DeviceState state);

    void addDevice(std::unique_ptr<Device> device);
    void addArrow(std::unique_ptr<Arrow> arrow);
    void addReceptionIcon(std::unique_ptr<ReceptionIcon> receptionIcon);
    void addDropAnimation(std::unique_ptr<PacketDrop> dropAnimation);
    void changeArrowState(int senderId, int receiverId, std::string state);
    std::pair<sf::Vector2f, bool> findDeviceCoordinates(int nodeId);

      // Animations
    void startBroadcast(const sf::Vector2f& startPosition, float duration);

    //Routing:
    void addRouting(int id1, int id2);
    void removeRouting(int id1, int id2);


    //metrics
    void addRetransmission(int nodeId);
    void incrementPacketSent(int nodeId);

    void incrementListeningData(int nodeId);
    void incrementTransmittingData(int nodeId);
    void incrementListeningAck(int nodeId);
    void incrementTransmittingAck(int nodeId);


    //todo: put this in priavte section and make getters/setters
    //UI:
    tgui::Gui& gui;
    tgui::TabContainer::Ptr tabContainer;
    tgui::Panel::Ptr serverPanel;
    tgui::Panel::Ptr networkPanel;
    tgui::Panel::Ptr logsPanel;
    tgui::Panel::Ptr metricsPanel;

    //Server TAB
    tgui::Label::Ptr serverStatusConnected;
    tgui::Label::Ptr serverStatusDisconnected;
    std::thread routineServer;
    std::atomic<bool> isRoutineServerRunning{false};
    void routineServerLoop();

    //Network TAB
    tgui::CanvasSFML::Ptr canvas;
    tgui::Label::Ptr timeText;
    //todo: put the buttons to handle routing display, time management
    tgui::Label::Ptr communicationModeText;

    //Logs TAB
    tgui::Label::Ptr logText;

    //Metrics TAB
    tgui::Label::Ptr nbRetransmissionText;
    tgui::Label::Ptr energyExpenditureText; 
    tgui::Label::Ptr pdrText;

};



#endif 
