#pragma once

#include "../Screen.hpp"
#include "VisualiserManager.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "ProtocolPacketController.hpp"
#include "ProtocolVisualisationState.hpp"



class ProtocolVisualisationScreen : public Screen {
    
public:
    explicit ProtocolVisualisationScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui);
    void handleEvent(InputManager& input) override;
    void update(float deltaTime,InputManager &input) override;
    void draw(sf::RenderWindow& window) override;
    void setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) override;

private:

    VisualiserManager manager;
    sf::View networkView;
    ProtocolVisualisationState state; 
    ProtocolPacketController packetController;
    void onResize() override;
};


