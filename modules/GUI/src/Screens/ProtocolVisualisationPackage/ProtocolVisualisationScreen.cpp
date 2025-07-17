#include "ProtocolVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../UI/UIFactory/UIFactory.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>


ProtocolVisualisationScreen::ProtocolVisualisationScreen(TcpServer& tcpServer, std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
    : Screen(gui),manager(state,gui),
      networkView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    tcpServer.setPacketHandler([this](sf::Packet& packet) {
        packetController.handlePacket(packet, state, manager);
    });

    


    setupUI(actions);
    manager.setupUI(networkView);

    //Todo: terminate the current process gracefully, right now there is memory problem which causes a crash
    //maybe you will want to have an "unfocused" state for the screen where packets are still received but not processed


}

void ProtocolVisualisationScreen::handleEvent(InputManager& input)
{

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

//todo: remove deltaTime in every update
void ProtocolVisualisationScreen::update(float deltaTime, InputManager& input)
{
    manager.update(input);
}

void ProtocolVisualisationScreen::draw(sf::RenderWindow& window)
{    
    manager.draw(window, networkView, state);

}

void ProtocolVisualisationScreen::setupUI(std::vector<std::pair<std::string, ScreenAction>> actions)
{
    gui.removeAllWidgets();
    updateTextSize(0.02f);  // Update text size based on new window dimensions
    auto picture = tgui::Picture::create("assets/background.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);

    
        auto button = UIFactory::createButton("Back", actions[0].second);
        button->setSize({"7%", "4%"});
        button->setPosition({"1%", "1%"});
        gui.add(button);
    
}

void ProtocolVisualisationScreen::onResize()
{
    updateTextSize(0.02f);  // Update text size based on new window dimensions
    auto size =manager.canvas->getSize();
    networkView.setSize(size);
    networkView.setCenter(size / 2.f);
    manager.canvas->setView(networkView);
}

