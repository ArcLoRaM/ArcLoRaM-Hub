


#include <iostream>
#include <thread>
#include "VisualiserManager/VisualiserManager.hpp"
#include "Common/Common.hpp"







inline  void displayThread(VisualiserManager& manager) {

    sf::Vector2u windowSize = sf::Vector2u(windowWidth, windowHeight);
    sf::RenderWindow window(sf::VideoMode(windowSize), "ArcLoRaM Protocol",sf::Style::Default);
    sf::Font font;

    if (!font.openFromFile("assets/arial.ttf")) {
        std::cerr << "Error loading font\n";
        isRunning = false;
        return;
    }


    while (window.isOpen() && isRunning) {
        
        //Deprecated
        // sf::Event event;

// "C++ lets you deduce the template parameter which is why you can write const std::optional event instead of
//  const std::optional<sf::Event> event. const auto event is another valid choice if you prefer a shorter expression."



        while (const std::optional event= window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                isRunning = false;
                window.close();
                
            }

           else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            if (keyPressed->scancode == sf::Keyboard::Scancode::Escape){
                isRunning = false;
                window.close();
                 }
            }

            // Handle button clicks
            for (auto& button : manager.buttons) {
                button->handleEvent(*event, window);
            }

            // Handle Device Clicks
            for(auto& device: manager.devices){
                device->handleEvent(*event,window);
            }
        }

        window.clear(sf::Color::Black);


        // Draw visualiser manager
        {
        std::lock_guard<std::mutex> lockDevice(deviceMutex);
        manager.update();
        manager.draw(window);
        }

        for (auto& button : manager.buttons) {
            button->draw( window);
        }

        {
        std::lock_guard<std::mutex> lock(logMutex);
        
        float y = 940.0f;
        while (logMessages.size() > 10) {
            logMessages.erase(logMessages.begin());
        }
        for (auto it = logMessages.rbegin(); it != logMessages.rend(); ++it) {
            sf::Text text(font,*it,10);
            text.setFillColor(sf::Color::White);
            text.setPosition(sf::Vector2f(10.0f, y));
            window.draw(text);
            y -= 15.0f;
        }
        }


        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


