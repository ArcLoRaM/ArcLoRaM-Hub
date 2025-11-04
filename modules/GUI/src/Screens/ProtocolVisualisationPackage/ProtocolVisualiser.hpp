#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include "UI/UIController.hpp"
#include "Managers/AnimationManager.hpp"
#include "Managers/RoutingManager.hpp"
#include "Managers/DeviceManager.hpp"
#include "Simulation/SimulationController.hpp"
#include "../../Network/Client/ClientConnectionMonitor.hpp"
#include "States/ProtocolVisualisationState.hpp"
#include "States/TopologyFileState.hpp"


class ProtocolVisualiser {
public:
    ProtocolVisualiser(tgui::Gui& gui);

    void setup(sf::View& view);
    void update(InputManager& inputManager);
    void draw(sf::RenderWindow& window, sf::View& view);

    DeviceManager& getDeviceManager() { return deviceManager; }
    RoutingManager& getRoutingManager() { return routingManager; }
    AnimationManager& getAnimationManager() { return animationManager; }
    ProtocolVisualisationState& getProtocolState() { return protocolState; }

    tgui::CanvasSFML::Ptr getCanvas() { return uiController.getCanvas(); }
private:
    tgui::Gui& gui;
    ProtocolVisualisationState protocolState;
    TopologyFileState topologyState;

    UIController uiController;
    AnimationManager animationManager;
    RoutingManager routingManager;
    DeviceManager deviceManager;
    SimulationController simulationController;
    ClientConnectionMonitor clientConnectionMonitor;
    void bindUIEvents();
};
