#include "Application.hpp"
#include "../Screens/HomePackage/HomePageScreen.hpp"
#include "../Screens/ProtocolVisualisationPackage/ProtocolVisualisationScreen.hpp"
#include <thread>
#include <atomic>
#include "../Shared/InputManager/InputManager.hpp"
#include "../Shared/Config.hpp"

Application::Application()
    : window(sf::VideoMode(sf::Vector2u(config::windowWidth, config::windowHeight)), "ArcLoRaM GUI", sf::Style::Default)
{
    tcpServer.start(5000);

    // Set the initial screen to the home page
auto homePageActions = std::make_shared<std::vector<std::pair<std::string, ScreenAction>>>();

*homePageActions = {
    { "New Topology", []() { /* Placeholder */ } },
    { "Edit Topology", []() { /* Placeholder */ } },
    { "Network Visualisation", [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
        };
        changeScreen(std::make_unique<ProtocolVisualisationScreen>(tcpServer, backToHome));
    }}
};

changeScreen(std::make_unique<HomePageScreen>(*homePageActions));
}

void Application::run()
{

    // remains scoped
    InputManager inputManager;

    sf::Clock mainClock;

    while (window.isOpen())
    {
        // "C++ lets you deduce the template parameter which is why you can write const std::optional event instead of
        //  const std::optional<sf::Event> event. const auto event is another valid choice if you prefer a shorter expression."
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            inputManager.handleEvent(event);

            currentScreen->handleEvent(inputManager);
        }

        float deltaTime = mainClock.restart().asSeconds();
        currentScreen->update(deltaTime, inputManager);
        window.clear();
        currentScreen->draw(window);
        window.display();

        // must be at the end of the frame
        inputManager.postUpdate(window);
    }
}

void Application::changeScreen(std::unique_ptr<Screen> newScreen)
{
    currentScreen = std::move(newScreen);
}

sf::RenderWindow &Application::getWindow()
{
    return window;
}
