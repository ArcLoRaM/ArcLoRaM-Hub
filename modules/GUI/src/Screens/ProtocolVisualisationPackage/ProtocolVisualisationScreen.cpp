#include "ProtocolVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../UI/UIFactory/UIFactory.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../Network/TcpServer/TcpServer.hpp"



ProtocolVisualisationScreen::ProtocolVisualisationScreen( std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
    : Screen(gui),visualiser(gui),
      networkView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight})),packetHandler(visualiser)
{
    auto& tcpServer = TcpServer::instance();
    tcpServer.setPacketHandler([this](sf::Packet& packet) {
        packetHandler.handlePacket(packet);
    });

    setupUI(actions);
    visualiser.setup(networkView);
    tcpServer.start(5000);


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

void ProtocolVisualisationScreen::update( InputManager& input)
{
    visualiser.update(input);
}

void ProtocolVisualisationScreen::draw(sf::RenderWindow& window)
{    
    visualiser.draw(window, networkView);

}

void ProtocolVisualisationScreen::setupUI(std::vector<std::pair<std::string, ScreenAction>> actions)
{
    gui.removeAllWidgets();
    updateTextSize(0.02f);  // Update text size based on new window dimensions
    auto picture = tgui::Picture::create("assets/background.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);


    //todo: undertand why it's laggy when you click back, probably not well handled memory leak...
    //todo: don't do a button, not very aesthetic...  
    auto button = UIFactory::createButton("Back", [this, actions]() {
            actions[0].second();
            TcpServer::instance().stop();

            
        });
        button->setSize({"7%", "4%"});
        button->setPosition({"1%", "1%"});
        gui.add(button);
    
}

void ProtocolVisualisationScreen::onResize()
{
    updateTextSize(0.02f);  // Update text size based on new window dimensions
    auto size =visualiser.getCanvas()->getSize();
    networkView.setSize(size);
    networkView.setCenter(size / 2.f);
    visualiser.getCanvas()->setView(networkView);
}

