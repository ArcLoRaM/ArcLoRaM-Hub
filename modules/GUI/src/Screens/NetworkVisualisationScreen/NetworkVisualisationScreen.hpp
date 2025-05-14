#pragma once

#include "Screen.hpp"
#include "../../Visualisation/VisualiserManager/VisualiserManager.hpp"

class NetworkVisualisationScreen : public Screen {
public:
    explicit NetworkVisualisationScreen(VisualiserManager& manager);
    void handleEvent(const std::optional<sf::Event> event) override;
    void update(float deltaTime) override;
    void draw(sf::RenderWindow& window) override;

private:
    VisualiserManager& manager;
    sf::View networkView;
    sf::Font* font; // Reference from ResourceManager
};
