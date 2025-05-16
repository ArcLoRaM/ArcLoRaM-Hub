#include "ProtocolVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"

ProtocolVisualisationScreen::ProtocolVisualisationScreen(TcpServer& tcpServer, ScreenAction backAction)
    : manager(),
      networkView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    tcpServer.setPacketHandler([this](sf::Packet& packet) {
        packetController.handlePacket(packet, state, manager);
    });

    float buttonWidth = 150.f;
    float buttonHeight = 50.f;
    float posX = config::windowWidth - buttonWidth - 20.f;  // Top-right corner, 20px margin
    float posY = 20.f;

    backButton = std::make_unique<Button>(
        posX,
        posY,
        buttonWidth,
        buttonHeight,
        sf::Color(200, 50, 50),
        "Back",
        "Arial"
    );
    backButton->setOnClick(backAction);
}

void ProtocolVisualisationScreen::handleEvent(InputManager& input)
{
    backButton->update(input);

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

void ProtocolVisualisationScreen::update(float deltaTime, InputManager& input)
{
    (void)deltaTime;
    manager.update(input);
}

void ProtocolVisualisationScreen::draw(sf::RenderWindow& window)
{
    manager.draw(window, networkView, state);
    backButton->draw(window);
}
