#include "NetworkVisualisationScreen.hpp"
#include "../../Shared/RessourceManager/RessourceManager.hpp"
#include "../../Shared/Common/Common.hpp"
//suppress RenderWindow when you have the new input system.
NetworkVisualisationScreen::NetworkVisualisationScreen(sf::RenderWindow &window)
    : manager(window),window(window),
      networkView(sf::FloatRect({0, 0}, {(float)windowWidth,(float) windowHeight})),
    networkManager(std::make_unique<NetworkManager>(manager,window))

{
    font = &ResourceManager::getInstance().getFont("Arial");
}

void NetworkVisualisationScreen::handleEvent(const std::optional<sf::Event> event)
{
    if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>())
    {

        switch (keyPressed->scancode) {
            case sf::Keyboard::Scancode::Left:
                networkView.move({-10.f, 0.f});
                break;
            case sf::Keyboard::Scancode::Right:
                networkView.move({10.f, 0.f});
                break;
            case sf::Keyboard::Scancode::Up:
                networkView.move({0.f, -10.f});
                break;
            case sf::Keyboard::Scancode::Down:
                networkView.move({0.f, 10.f});
                break;
            default:
                break;
        }
    }

    else if (const auto *mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
    {
        switch (mouseWheelScrolled->wheel)
        {
        case sf::Mouse::Wheel::Vertical:
            if (mouseWheelScrolled->delta > 0)
            {
                networkView.zoom(0.9f); // Zoom in
            }
            else
            {
                networkView.zoom(1.1f); // Zoom out
            }
            break;
        }

        // Buttons
        for (auto &button : manager.buttons)
        {
            button->handleEvent(*event); // Pass window reference if needed
        }

        // Devices
        for (auto &device : manager.devices)
        {
            device->handleEvent(*event);
        }
    }
}

void NetworkVisualisationScreen::update(float deltaTime)
{
    std::lock_guard<std::mutex> lockDevice(deviceMutex);
    manager.update();
}

void NetworkVisualisationScreen::draw(sf::RenderWindow &window)
{

    window.setView(window.getDefaultView());

    // Draw buttons (static UI)
    for (auto &button : manager.buttons)
    {
        button->draw(window);
    }

    // Draw logs (temporary retained from your code)
    {
        std::lock_guard<std::mutex> lock(logMutex);

        float y = 940.0f;
        while (logMessages.size() > 10)
            logMessages.erase(logMessages.begin());

        for (auto it = logMessages.rbegin(); it != logMessages.rend(); ++it)
        {
            sf::Text text(*font, *it, 10);
            text.setFillColor(sf::Color::White);
            text.setPosition(sf::Vector2f(10.0f, y));
            window.draw(text);
            y -= 15.0f;
        }
    }

    window.setView(networkView);
    manager.draw(window);
}
