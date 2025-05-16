#pragma once

#include <SFML/Graphics.hpp>
#include "../Screens/Screen.hpp"
#include <memory>
#include "../Network/TcpServer/TcpServer.hpp"
#include "../Shared/InputManager/InputManager.hpp"

class Application {
public:
    Application();
    void run();

    void changeScreen(std::unique_ptr<Screen> newScreen);

    sf::RenderWindow& getWindow();

private:
    sf::RenderWindow window;
    std::unique_ptr<Screen> currentScreen;
    TcpServer tcpServer; // Owns the server, lives as long as the application
    InputManager inputManager;
};
