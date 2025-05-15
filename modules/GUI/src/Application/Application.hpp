#pragma once

#include <SFML/Graphics.hpp>
#include "../Screens/Screen.hpp"
#include <memory>

class Application {
public:
    Application();
    void run();

    void changeScreen(std::unique_ptr<Screen> newScreen);

    sf::RenderWindow& getWindow();

private:
    sf::RenderWindow window;
    std::unique_ptr<Screen> currentScreen;
};
