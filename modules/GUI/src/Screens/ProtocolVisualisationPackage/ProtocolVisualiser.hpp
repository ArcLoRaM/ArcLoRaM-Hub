#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include "UI/UIController.hpp"
#include "Managers/AnimationManager.hpp"
#include "Managers/RoutingManager.hpp"
#include "Managers/DeviceManager.hpp"
#include "Simulation/SimulationController.hpp"
#include "../../Network/Client/ClientConnectionMonitor.hpp"
#include "States/LiveNetworkState.hpp"
#include "States/SimulationConfiguration.hpp"


class ProtocolVisualiser {
public:
    ProtocolVisualiser(tgui::Gui& gui);

    void setup(sf::View& view);
    void update(InputManager& inputManager);
    void draw(sf::RenderWindow& window, sf::View& view);

    DeviceManager& getDeviceManager() { return deviceManager; }
    RoutingManager& getRoutingManager() { return routingManager; }
    AnimationManager& getAnimationManager() { return animationManager; }
    LiveNetworkState& getProtocolState() { return protocolState; }

    tgui::CanvasSFML::Ptr getCanvas() { return uiController.getCanvas(); }

    void findNode(const std::string& searchQuery);
private:
    tgui::Gui& gui;
    sf::View* networkView = nullptr;
    LiveNetworkState protocolState;
    SimulationConfiguration topologyState;

    UIController uiController;
    AnimationManager animationManager;
    RoutingManager routingManager;
    DeviceManager deviceManager;
    SimulationController simulationController;
    ClientConnectionMonitor clientConnectionMonitor;
    void bindUIEvents();
};
