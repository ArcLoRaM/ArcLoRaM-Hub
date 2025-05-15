#include "Application.hpp"
#include "../Screens/NetworkVisualisationScreen/NetworkVisualisationScreen.hpp" // Initial screen
#include <thread>
#include <atomic>

Application::Application()
    : window( sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), "ArcLoRaM GUI",sf::Style::Default)
{
    // Start at HomeScreen
    changeScreen(std::make_unique<NetworkVisualisationScreen>(window));

}

void Application::run() {



    sf::Clock mainClock;
    while (window.isOpen()) {
        // "C++ lets you deduce the template parameter which is why you can write const std::optional event instead of
        //  const std::optional<sf::Event> event. const auto event is another valid choice if you prefer a shorter expression."
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            currentScreen->handleEvent(event);
        }

        float deltaTime = mainClock.restart().asSeconds();
        currentScreen->update(deltaTime);
        window.clear();
        currentScreen->draw(window);
        window.display();
        
        //delay to limit the frame rate
        //TODO
    }
}

void Application::changeScreen(std::unique_ptr<Screen> newScreen) {
    currentScreen = std::move(newScreen);
}

sf::RenderWindow& Application::getWindow() {
    return window;
}
