#pragma once

#include <functional>
#include <string>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../States/LiveNetworkState.hpp"
class UIController {
public:
    UIController(tgui::Gui& gui);

    void setupUI(sf::View& networkView);

    void updateUI(LiveNetworkState& state);
    //Probably better to make functions that correspond to workflow instead of manually setting each element
    //example: simulationStartUIChanges(), simulationPauseUIChanges()
    void setServerStatus(bool connected);
    void setFileNameLabel(const std::string& name);


    void enableStartButton(bool enabled);

    // External callbacks
    std::function<void()> onStartSimulation;
    std::function<void()> onStopSimulation;
    std::function<void()> onRestartSimulation;
    std::function<void()> onPauseSimulation;
    std::function<void()> onResumeSimulation;
    std::function<void(const int scenarioId)> onScenarioTypeChanged;
    std::function<void(const std::string& filePath)> onFileSelected;
    std::function<void(const std::string& searchQuery)> onFindNode;


    bool getRoutingDisplayEnabled() const { return routingDisplayEnabled; }

    tgui::CanvasSFML::Ptr getCanvas()  { return canvas; }

    void errorMessageBox(const std::string& message);

private:

    void bindCallbacks();

    void setClientPanelUI();
    void setProtocolPanelUI(sf::View &networkView);
    void setLogsPanelUI();
    void setMetricsPanelUI();

    void hideAllSimulationControlButtons();

    void showSimulationStartedUI();
    void showSimulationRunningUI();
    void showSimulationPausedUI();
    void showSimulationStoppedUI();
    void showSimulationRestartedUI();
    void showSimulationReadyToStartUI();

    tgui::Gui& gui;
    tgui::TabContainer::Ptr tabContainer;

    //CLIENT TAB
    tgui::Panel::Ptr clientPanel;
    tgui::Button::Ptr startSimulationButton;
    tgui::Button::Ptr stopSimulationButton;
    tgui::Button::Ptr restartSimulationButton;
    tgui::Button::Ptr confFileSelectionButton;
    tgui::Group::Ptr confFileSelectionGroup;
    tgui::Label::Ptr fileNameLabel;
    tgui::Label::Ptr serverStatusConnected;
    tgui::Label::Ptr serverStatusDisconnected;
    tgui::ComboBox::Ptr scenarioTypeComboBox;


    //Network TAB
    tgui::Button::Ptr pauseSimulationButton;
    tgui::Button::Ptr resumeSimulationButton;
    tgui::Panel::Ptr networkPanel;
    tgui::CanvasSFML::Ptr canvas;
    tgui::Label::Ptr communicationModeText;
    tgui::Label::Ptr timeText;
    tgui::EditBox::Ptr searchBox;
    tgui::Button::Ptr findButton;


    //LOG TAB
    tgui::Panel::Ptr logsPanel;

    //METRIC TAB
    tgui::Panel::Ptr metricsPanel;

    bool routingDisplayEnabled = false; // Flag to control routing display

};
