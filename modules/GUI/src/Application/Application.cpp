#include "Application.hpp"
#include "../Screens/ProtocolVisualisationPackage/ProtocolVisualisationScreen.hpp" // Initial screen
#include <thread>
#include <atomic>
#include "../Shared/InputManager/InputManager.hpp"
#include "../Shared/Config.hpp"

Application::Application()
    : window( sf::VideoMode(sf::Vector2u(config::windowWidth, config::windowHeight)), "ArcLoRaM GUI",sf::Style::Default)
{
       tcpServer.start(5000);
    // Start at HomeScreen
    changeScreen(std::make_unique<ProtocolVisualisationScreen>(tcpServer));

}

void Application::run() {

    //remains scoped
    InputManager inputManager;

    sf::Clock mainClock;
    while (window.isOpen()) {
        // "C++ lets you deduce the template parameter which is why you can write const std::optional event instead of
        //  const std::optional<sf::Event> event. const auto event is another valid choice if you prefer a shorter expression."
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
                
                inputManager.handleEvent(event);

            currentScreen->handleEvent(inputManager);
        }
        
        inputManager.postUpdate(window);

        float deltaTime = mainClock.restart().asSeconds();
        currentScreen->update(deltaTime);
        window.clear();
        currentScreen->draw(window);
        window.display();
        
   
    }
}

void Application::changeScreen(std::unique_ptr<Screen> newScreen) {
    currentScreen = std::move(newScreen);
}

sf::RenderWindow& Application::getWindow() {
    return window;
}
