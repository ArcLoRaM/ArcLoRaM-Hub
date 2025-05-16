#include "HomePageScreen.hpp"
#include "../../Shared/Config.hpp"

HomePageScreen::HomePageScreen()
    : view(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    float buttonWidth = 300.f;
    float buttonHeight = 100.f;
    float spacing = 50.f;
    float startX = (config::windowWidth - buttonWidth) / 2.f;
    float startY = (config::windowHeight - (3 * buttonHeight + 2 * spacing)) / 2.f;

    auto createButton = [&](const std::string& text, float yPos) {
        auto btn = std::make_unique<Button>(startX, yPos, buttonWidth, buttonHeight, sf::Color(50, 150, 250), text, "Arial");
        btn->setOnClick([]() { /* empty for now */ });
        return btn;
    };

    buttons.push_back(createButton("New Topology", startY));
    buttons.push_back(createButton("Edit Topology", startY + buttonHeight + spacing));
    buttons.push_back(createButton("Network Visualisation", startY + 2 * (buttonHeight + spacing)));
}

void HomePageScreen::handleEvent(InputManager& input)
{
    // Possible future use (navigation, etc.)
}

void HomePageScreen::update(InputManager& input)
{
    for (auto& btn : buttons)
        btn->update(input);
}

void HomePageScreen::draw(sf::RenderWindow& window)
{
    window.setView(view);
    for (auto& btn : buttons)
        btn->draw(window);
}
