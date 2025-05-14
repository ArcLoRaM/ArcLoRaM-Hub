

#include <iostream>
#include <thread>
#include "VisualiserManager/VisualiserManager.hpp"
#include "../Shared/Common/Common.hpp"
#include "../Shared/RessourceManager/RessourceManager.hpp"

inline void displayThread(VisualiserManager &manager)
{

    sf::Vector2u windowSize = sf::Vector2u(windowWidth, windowHeight);
    sf::RenderWindow window(sf::VideoMode(windowSize), "ArcLoRaM GUI", sf::Style::Default);
    sf::Font font;

    //Content View:
    sf::View networkView(window.getDefaultView()); // Copy default view initially



    //Todo: centralize Font access (right now other classes are calling openFromFile too)
    if (!font.openFromFile("assets/arial.ttf"))
    {
        std::cerr << "Error loading font\n";
        isRunning = false;
        return;
    }
    

    while (window.isOpen() && isRunning)
    {

        // "C++ lets you deduce the template parameter which is why you can write const std::optional event instead of
        //  const std::optional<sf::Event> event. const auto event is another valid choice if you prefer a shorter expression."
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                isRunning = false;
                window.close();
            }

            else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                {
                    isRunning = false;
                    window.close();
                }

                else if (keyPressed->scancode == sf::Keyboard::Scancode::Left)
                {
                    networkView.move(sf::Vector2f(-10, 0)); // Move left
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Right)
                {
                    networkView.move(sf::Vector2f(10, 0)); // Move right
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Up)
                {
                    networkView.move(sf::Vector2f(0, -10)); // Move up
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Down)
                {
                    networkView.move(sf::Vector2f(0, 10)); // Move down
                }
            }

            else if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                switch (mouseWheelScrolled->wheel)
                {
                    case sf::Mouse::Wheel::Vertical:
                        if(mouseWheelScrolled->delta > 0)
                        {
                            networkView.zoom(0.9f); // Zoom in
                        }
                        else
                        {
                            networkView.zoom(1.1f); // Zoom out
                        }
                        break;

                }
            }

            

            // Handle button clicks
            for (auto &button : manager.buttons)
            {
                button->handleEvent(*event, window);
            }

            // Handle Device Clicks
            for (auto &device : manager.devices)
            {
                device->handleEvent(*event, window);
            }
        }

        window.clear(sf::Color::Black);

        // Drawing Elements------------------------------------------------------------------

        {
            std::lock_guard<std::mutex> lockDevice(deviceMutex);
            window.setView(networkView); // Set the view to the network view

            // Update dynamic UI elements states (nodes, packet flowing etc...) with recent time values and new information from the simulator
            manager.update();
            // Draw the dynamic components of the visualiser manager
            manager.draw(window);
        }

        // draw the static UI elements (buttons, chat etc...)
        {
            window.setView(window.getDefaultView());
            //Buttons
            for (auto &button : manager.buttons)
            {
                button->draw(window);
            }

            {
                std::lock_guard<std::mutex> lock(logMutex);

                float y = 940.0f;
                while (logMessages.size() > 10)
                {
                    logMessages.erase(logMessages.begin());
                }
                for (auto it = logMessages.rbegin(); it != logMessages.rend(); ++it)
                {
                    sf::Text text(font, *it, 10);
                    text.setFillColor(sf::Color::White);
                    text.setPosition(sf::Vector2f(10.0f, y));
                    window.draw(text);
                    y -= 15.0f;
                }
            }

            window.display();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
}
