


#include <iostream>
#include <thread>
#include "VisualiserManager/VisualiserManager.hpp"
#include "Common/Common.hpp"







inline  void displayThread(VisualiserManager& manager) {
    sf::RenderWindow window(sf::VideoMode(windowWidth, windowHeight), "ArcLoRaM Protocol",sf::Style::Default);
    sf::Font font;
    if (!font.loadFromFile("assets/arial.ttf")) {
        std::cerr << "Error loading font\n";
        isRunning = false;
        return;
    }


    while (window.isOpen() && isRunning) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                isRunning = false;
                window.close();
                
            }

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Q) {
                isRunning = false;
                window.close();
            }

            // Handle button clicks
            for (auto& button : manager.buttons) {
                button->handleEvent(event, window);
            }

            // Handle Device Clicks
            for(auto& device: manager.devices){
                device->handleEvent(event,window);
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
            sf::Text text(*it, font, 10);
            text.setFillColor(sf::Color::White);
            text.setPosition(10.0f, y);
            window.draw(text);
            y -= 15.0f;
        }
        }


        window.display();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


