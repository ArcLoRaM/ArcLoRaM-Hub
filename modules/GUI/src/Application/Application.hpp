#pragma once

#include <SFML/Graphics.hpp>
#include "../Screens/Screen.hpp"
#include <memory>
#include "../Network/TcpServer/TcpServer.hpp"
#include "../Shared/InputManager/InputManager.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>

class Application {
public:
    Application();
    void run();

    void changeScreen(std::unique_ptr<Screen> newScreen);

    sf::RenderWindow& getWindow();

private:
    sf::RenderWindow window;
    std::unique_ptr<Screen> currentScreen;
    std::unique_ptr<Screen> nextScreen;

    TcpServer tcpServer; // Owns the server, lives as long as the application
    InputManager inputManager;

    tgui::Gui gui; // TGUI GUI object
};
