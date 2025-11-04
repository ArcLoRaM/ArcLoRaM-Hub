#pragma once

#include "../Screen.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "PacketHandler/PacketHandler.hpp"
#include "ProtocolVisualiser.hpp"



class ProtocolVisualisationScreen : public Screen {
    
    /*
    Landing Page. Owns manager, view, packet controller and state.
    Deal with high level input (back to main menu, zoom in/out, pan etc..).
    */
public:
    explicit ProtocolVisualisationScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui);
    void handleEvent(InputManager& input) override;
    void update(InputManager &input) override;
    void draw(sf::RenderWindow& window) override;
    void setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) override;

private:

    ProtocolVisualiser visualiser;
    sf::View networkView;
    PacketHandler packetHandler;
    void onResize() override;
};


