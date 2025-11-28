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
    void errorMessageBox(const std::string& message);


    //--------------------CLIENT TAB--------------------
    void setServerStatus(bool connected);
    void setFileNameLabel(const std::string& name);
    void enableStartButton(bool enabled);
    uint64_t getMaxSimulationTimeMs() const;

    std::function<void()> onStartSimulation;
    std::function<void()> onStopSimulation;
    std::function<void()> onRestartSimulation;
    std::function<void(const int scenarioId)> onScenarioTypeChanged;
    std::function<void(const std::string& filePath)> onFileSelected;
    std::function<void(const std::string& searchQuery)> onFindNode;

    void setBackButtonCallback(const std::function<void()>& callback) {
        backButton->onPress(callback);
    }

    //--------------------Network TAB--------------------
    std::function<void()> onPauseSimulation;
    std::function<void()> onResumeSimulation;
    bool getRoutingDisplayEnabled() const { return routingDisplayEnabled; }
    tgui::CanvasSFML::Ptr getCanvas()  { return canvas; }


    //-------------------LOG TAB--------------------





private:

    void bindCallbacks();
    void hideAllSimulationControlButtons();

    //Client
    void setClientPanelUI();
    void showSimulationStartedUI();
    void showSimulationStoppedUI();
    void showSimulationRestartedUI();
    void showSimulationReadyToStartUI();

    //Network
    void setProtocolPanelUI(sf::View &networkView);
    void showSimulationRunningUI();
    void showSimulationPausedUI();



    //Logs
    void setLogsPanelUI();
   
   
    tgui::Gui& gui;
    tgui::TabContainer::Ptr tabContainer;

    //--------------------CLIENT TAB--------------------
    tgui::Panel::Ptr clientPanel;
    //Todo: keep only one?
    tgui::Label::Ptr serverStatusConnected;
    tgui::Label::Ptr serverStatusDisconnected;

    tgui::Button::Ptr confFileSelectionButton;
    tgui::Panel::Ptr confFileSelectionGroup;
    tgui::Label::Ptr fileNameLabel;
    tgui::ComboBox::Ptr scenarioTypeComboBox;
    tgui::CheckBox::Ptr unlimitedSimulationCheckbox;
    tgui::Label::Ptr maxSimulationTimeLabel;
    tgui::EditBox::Ptr maxSimulationTimeInput;

    tgui::Panel::Ptr simulationCommandPanel;
    tgui::Button::Ptr backButton;
     tgui::Button::Ptr startSimulationButton;
    tgui::Button::Ptr stopSimulationButton;
    tgui::Button::Ptr restartSimulationButton;

    //--------------------Network TAB--------------------
    tgui::Panel::Ptr networkPanel;

    bool routingDisplayEnabled = false; // Flag to control routing display


    // TOP Layer
    tgui::Panel::Ptr topLayerPanel;
    tgui::BitmapButton::Ptr pauseBitmapButton;
    tgui::BitmapButton::Ptr resumeBitmapButton;
    tgui::BitmapButton::Ptr normalSpeedBitmapButton;
    tgui::BitmapButton::Ptr mediumSpeedBitmapButton;
    tgui::BitmapButton::Ptr fastSpeedBitmapButton;
    tgui::BitmapButton::Ptr fastForwardToNextEventButton;
    tgui::Button::Ptr exportSimulationButton;
    tgui::ProgressBar::Ptr simulationTimeProgressBar;
    tgui::RichTextLabel::Ptr tickInfoLabel;
    tgui::RichTextLabel::Ptr currentSimulationTimeLabel;

    //nodeBrowserPanel
    tgui::Panel::Ptr nodeBrowserPanel;
    tgui::EditBox::Ptr searchBox;
    tgui::Button::Ptr findButton;



    //Canvas
    tgui::CanvasSFML::Ptr canvas;


    //--------------------LOG TAB--------------------
    tgui::Panel::Ptr logsPanel;




};
