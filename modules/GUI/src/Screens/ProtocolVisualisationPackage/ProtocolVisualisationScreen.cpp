#include "ProtocolVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Config.hpp"
#include "../../Shared/UIFactory/UIFactory.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../Network/TcpServer/TcpServer.hpp"



ProtocolVisualisationScreen::ProtocolVisualisationScreen( std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui, InputManager& inputManager)
    : Screen(gui),visualiser(gui, inputManager),
      networkView(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight})),packetHandler(visualiser)
{
    auto& tcpServer = TcpServer::instance();
    tcpServer.setPacketHandler([this](sf::Packet& packet) {
        packetHandler.handlePacket(packet);
    });

    setupUI(actions);
    visualiser.setup(networkView);
    tcpServer.start(5000);

    //todo: undertand why it's laggy when you click back, probably not well handled memory leak...
    visualiser.setBackButtonCallback([actions]() {
        actions[0].second();
        TcpServer::instance().stop();
    });

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

    // Mouse drag panning
    auto canvas = visualiser.getCanvas();

    // Get mouse position in canvas world coordinates (4-step conversion pattern)
    sf::Vector2i pixelPos = input.getMouseScreenPosition();
    sf::Vector2f guiPos = gui.mapPixelToCoords(pixelPos);
    sf::Vector2f canvasLocal = guiPos - sf::Vector2f(canvas->getAbsolutePosition());
    sf::Vector2f mouseWorldPos = canvas->mapPixelToCoords(canvasLocal);

    // Check if mouse is within canvas bounds
    tgui::Vector2f canvasPos = canvas->getAbsolutePosition();
    tgui::Vector2f canvasSize = canvas->getSize();
    bool isMouseOverCanvas = (guiPos.x >= canvasPos.x && guiPos.x <= canvasPos.x + canvasSize.x &&
                              guiPos.y >= canvasPos.y && guiPos.y <= canvasPos.y + canvasSize.y);

    // Handle drag lifecycle
    if (input.isMouseButtonPressed(sf::Mouse::Button::Left) && isMouseOverCanvas) {
        if (!isDragging) {
            // Start dragging
            isDragging = true;
            lastMouseWorldPos = mouseWorldPos;
        } else {
            // Update view while dragging
            sf::Vector2f delta = lastMouseWorldPos - mouseWorldPos;
            networkView.move(delta);
            lastMouseWorldPos = mouseWorldPos;
        }
    } else {
        // End dragging
        isDragging = false;
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



    
}

void ProtocolVisualisationScreen::onResize()
{
    updateTextSize(0.02f);  // Update text size based on new window dimensions
    auto size =visualiser.getCanvas()->getSize();
    networkView.setSize(size);
    networkView.setCenter(size / 2.f);
    visualiser.getCanvas()->setView(networkView);
}

