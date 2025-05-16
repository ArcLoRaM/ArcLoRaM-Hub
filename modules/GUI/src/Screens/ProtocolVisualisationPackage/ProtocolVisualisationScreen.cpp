#include "ProtocolVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"

ProtocolVisualisationScreen::ProtocolVisualisationScreen(TcpServer& tcpServer)
    : manager(),
      networkView(sf::FloatRect({0, 0}, {(float)config::windowWidth,(float) config::windowHeight}))
 

{




        tcpServer.setPacketHandler([this](sf::Packet& packet) {
        packetController.handlePacket(packet, state, manager);
    });
}

void ProtocolVisualisationScreen::handleEvent(InputManager& input)
{
    //Change global Screen Information 

    // Move the view of the topology network
    if (input.isKeyPressed(sf::Keyboard::Scancode::Left)) {
        networkView.move({-10.f, 0.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Right)) {
        networkView.move({10.f, 0.f});
    }

    if (input.isKeyPressed(sf::Keyboard::Scancode::Up)) {
        networkView.move({0.f, -10.f});
    }
    if (input.isKeyPressed(sf::Keyboard::Scancode::Down)) {
        networkView.move({0.f, 10.f});
    }

    float wheelDelta = input.getMouseWheelDelta();
    if (wheelDelta > 0.f) {
        networkView.zoom(0.9f);
    }
    else if (wheelDelta < 0.f) {
        networkView.zoom(1.1f);
    }
    
}

void ProtocolVisualisationScreen::update(float deltaTime, InputManager &input)
{
    //deltatime is not used in this version as every animation have their own clock, but could be !
    manager.update(input);
}

void ProtocolVisualisationScreen::draw(sf::RenderWindow &window)
{

   
    manager.draw(window,networkView,state);
}
