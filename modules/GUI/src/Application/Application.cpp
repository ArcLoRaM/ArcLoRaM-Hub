#include "Application.hpp"
#include <iostream>
#include <SFML/Graphics.hpp>

// Application::Application()
//     : window(sf::Vector2u(windowWidth, windowHeight), "ArcLoRaM GUI")
// {
//     // Start with home screen
//     currentScreen = std::make_unique<HomeScreen>(*this);
// }

// void Application::run() {
//     sf::Clock clock;
//     while (window.isOpen()) {
//         sf::Event event;
//         while (window.pollEvent(event)) {
//             if (event.type == sf::Event::Closed)
//                 window.close();

//             handleEvent(event);
//         }

//         float deltaTime = clock.restart().asSeconds();

//         currentScreen->update(deltaTime);

//         window.clear();
//         currentScreen->draw(window);
//         window.display();
//     }
// }

// void Application::handleEvent(const sf::Event& event) {
//     currentScreen->handleEvent(event);
// }

// void Application::changeScreen(std::unique_ptr<Screen> newScreen) {
//     currentScreen = std::move(newScreen);
// }
