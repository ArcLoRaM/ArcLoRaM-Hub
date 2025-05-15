#pragma once

#include "../Screen.hpp"
#include "../../Visualisation/VisualiserManager/VisualiserManager.hpp"
#include "../../Network/NetworkManager/NetworkManager.hpp"
class NetworkVisualisationScreen : public Screen {
public:
    explicit NetworkVisualisationScreen(sf::RenderWindow& window);
    void handleEvent(const std::optional<sf::Event> event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    //suppress RenderWindow when you have the new inpuut system.
    sf::RenderWindow& window;
    
    VisualiserManager manager;
    //exclusive ownership and automatic cleanup of NetworkManager when the NetworkVisualisationScreen is destroyed,
    //preventing memory leaks and enforcing RAII
    std::unique_ptr<NetworkManager> networkManager;
    sf::View networkView;
    sf::Font* font; // Reference from ResourceManager
};
