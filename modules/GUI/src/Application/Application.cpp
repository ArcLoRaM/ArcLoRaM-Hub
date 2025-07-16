#include "Application.hpp"
#include "../Screens/HomePackage/HomePageScreen.hpp"
#include "../Screens/ProtocolVisualisationPackage/ProtocolVisualisationScreen.hpp"
#include "../Screens/TopologyEditorPackage/TopologyEditorScreen.hpp"
#include <thread>
#include <atomic>
#include "../Shared/InputManager/InputManager.hpp"
#include "../Shared/Config.hpp"
#include "../UI/UIFactory/UIFactory.hpp"


Application::Application()
    : window(sf::VideoMode(sf::Vector2u(config::windowWidth, config::windowHeight)), "ArcLoRaM"),
    gui(window),theme() // Initialize TGUI with window
{
    
    theme.load("assets/themes/BabyBlue.txt");
    UIFactory::setTheme(&theme);

    tcpServer.start(5000);

auto homePageActions = std::make_shared<std::vector<std::pair<std::string, ScreenAction>>>();
//provision the actions for the home page
*homePageActions = {
    { "New Topology",  [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions,gui));
        };
        std::vector<std::pair<std::string, ScreenAction>> actions = {
            { "Back", backToHome }
        };
        changeScreen(std::make_unique<TopologyEditorScreen>(actions,gui));
    }},
    { "Edit Topology", [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions,gui));
        };
        std::vector<std::pair<std::string, ScreenAction>> actions = {
            { "Back", backToHome }
        };
       changeScreen(std::make_unique<TopologyEditorScreen>( actions,gui,"Edit"));
    }},
    { "Network Visualisation", [this, homePageActions]() {
        auto backToHome = [this, homePageActions]() {
            changeScreen(std::make_unique<HomePageScreen>(*homePageActions,gui));
        };
        changeScreen(std::make_unique<ProtocolVisualisationScreen>(tcpServer, backToHome,gui));
    }}
};
    // Set the initial screen to the home page
changeScreen(std::make_unique<HomePageScreen>(*homePageActions,gui));

//To call the appropriate resize function when the window is resized
gui.onViewChange([this] {
    if (currentScreen)
        currentScreen->onResize();  // forward to current screen
});


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
            gui.handleEvent(*event);

            if (currentScreen)
                currentScreen->handleEvent(inputManager);
        }

        float deltaTime = mainClock.restart().asSeconds();

        if (currentScreen) {
            currentScreen->update(deltaTime, inputManager);

            window.clear();
            currentScreen->draw(window);
            gui.draw();
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

tgui::Gui& Application::getGui()
{
    return gui;
}