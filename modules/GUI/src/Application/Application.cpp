#include "Application.hpp"
#include "../Screens/HomePackage/HomePageScreen.hpp"
#include "../Screens/ProtocolVisualisationPackage/ProtocolVisualisationScreen.hpp"
#include "../Screens/TopologyEditorPackage/TopologyEditorScreen.hpp"
#include <thread>
#include <atomic>
#include "../Shared/InputManager/InputManager.hpp"
#include "../Shared/Config.hpp"

Application::Application()
    : window(sf::VideoMode(sf::Vector2u(config::windowWidth, config::windowHeight)), "ArcLoRaM GUI", sf::Style::Default)
{
    tcpServer.start(5000);

auto homePageActions = std::make_shared<std::vector<std::pair<std::string, ScreenAction>>>();
//provision the actions for the home page
*homePageActions = {
    { "New Topology",  [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
        };
        changeScreen(std::make_unique<TopologyEditorScreen>(backToHome));
    }},
    { "Edit Topology", [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
        };
       changeScreen(std::make_unique<TopologyEditorScreen>("output/topology_config.txt", backToHome));
    }},
    { "Network Visualisation", [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
        };
        changeScreen(std::make_unique<ProtocolVisualisationScreen>(tcpServer, backToHome));
    }}
};
    // Set the initial screen to the home page
changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
std::cout << "Application object created" << std::endl;
}
void Application::run()
{
    InputManager inputManager;
    sf::Clock mainClock;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            inputManager.handleEvent(event);

            if (currentScreen)
                currentScreen->handleEvent(inputManager);
        }

        float deltaTime = mainClock.restart().asSeconds();

        if (currentScreen) {
            currentScreen->update(deltaTime, inputManager);

            window.clear();
            currentScreen->draw(window);
            window.display();
        }

        inputManager.postUpdate(window);
        if (nextScreen) {
             currentScreen = std::move(nextScreen);
}
    }
}

void Application::changeScreen(std::unique_ptr<Screen> newScreen)
{
    nextScreen = std::move(newScreen);
}

sf::RenderWindow &Application::getWindow()
{
    return window;
}
