#pragma once

#include <functional>
#include <string>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../States/ProtocolVisualisationState.hpp"
class UIController {
public:
    UIController(tgui::Gui& gui);

    void setupUI(sf::View& networkView);

    void updateUI(ProtocolVisualisationState& state);
    //Probably better to make functions that correspond to workflow instead of manually setting each element
    //example: simulationStartUIChanges(), simulationPauseUIChanges()
    void setServerStatus(bool connected);
    void setFileNameLabel(const std::string& name);
    void setPauseResumeText(const std::string& text);


    void enableStartButton(bool enabled);
    void enablePauseResumeButton(bool enabled);

    // External callbacks
    std::function<void()> onStartSimulation;
    std::function<void()> onPauseResumeSimulation;
    std::function<void(const std::string& filePath)> onFileSelected;


    bool getRoutingDisplayEnabled() const { return routingDisplayEnabled; }

    tgui::CanvasSFML::Ptr getCanvas()  { return canvas; }

    void errorMessageBox(const std::string& message);

private:

    void bindCallbacks();

    void setNetworkPanelUI(sf::View &networkView);
    void setLogsPanelUI();
    void setMetricsPanelUI();
    void setServerPanelUI();
    
    tgui::Gui& gui;
    tgui::TabContainer::Ptr tabContainer;

    //SERVER TAB
    tgui::Panel::Ptr serverPanel;
    tgui::Button::Ptr startSimulationButton;
    tgui::Button::Ptr pauseResumeSimulationButton;
    tgui::Button::Ptr confFileSelectionButton;
    tgui::Group::Ptr confFileSelectionGroup;
    tgui::Label::Ptr fileNameLabel;
    tgui::Label::Ptr serverStatusConnected;
    tgui::Label::Ptr serverStatusDisconnected;


    //Network TAB
    tgui::Panel::Ptr networkPanel;
    tgui::CanvasSFML::Ptr canvas;
    tgui::Label::Ptr communicationModeText;
    tgui::Label::Ptr timeText;


    //LOG TAB
    tgui::Panel::Ptr logsPanel;

    //METRIC TAB
    tgui::Panel::Ptr metricsPanel;

    bool routingDisplayEnabled = false; // Flag to control routing display

};
