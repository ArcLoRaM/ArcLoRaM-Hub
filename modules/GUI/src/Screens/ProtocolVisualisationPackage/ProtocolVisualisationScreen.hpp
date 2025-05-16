#pragma once

#include "../Screen.hpp"
#include "VisualiserManager.hpp"
#include "../../Shared/InputManager/InputManager.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"
#include "ProtocolPacketController.hpp"
#include "ProtocolVisualisationState.hpp"



class ProtocolVisualisationScreen : public Screen {
    
public:
    explicit ProtocolVisualisationScreen(TcpServer& tcpServer,ScreenAction backAction);
    void handleEvent(InputManager& input) override;
    void update(float deltaTime,InputManager &input) override;
    void draw(sf::RenderWindow& window) override;

private:

    
    VisualiserManager manager;
    sf::View networkView;
    ProtocolVisualisationState state; 
    ProtocolPacketController packetController;
    std::unique_ptr<Button> backButton;

};


