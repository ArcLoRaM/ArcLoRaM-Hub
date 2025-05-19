#include "HomePageScreen.hpp"
#include "../../Shared/Config.hpp"

HomePageScreen::HomePageScreen(std::vector<std::pair<std::string, ScreenAction>> actions)
    : view(sf::FloatRect({0, 0}, {(float)config::windowWidth, (float)config::windowHeight}))
{
    float buttonWidth = 300.f;
    float buttonHeight = 100.f;
    float spacing = 50.f;
    float startX = (config::windowWidth - buttonWidth) / 2.f;
    float startY = (config::windowHeight - (actions.size() * buttonHeight + (actions.size() - 1) * spacing)) / 2.f;


    //Create the three buttons from the actions variable : title -lambda
    for (std::size_t i = 0; i < actions.size(); ++i) {
        const auto& [label, action] = actions[i];

        auto btn = std::make_unique<Button>(
            startX,
            startY + i * (buttonHeight + spacing),
            buttonWidth,
            buttonHeight,
            sf::Color(50, 150, 250),
            label,
            "Arial",
            false
        );
        btn->setOnClick(action);
        buttons.push_back(std::move(btn));
    }
}

void HomePageScreen::handleEvent(InputManager& input) {
    // Possible future use (navigation, etc.)
}

void HomePageScreen::update(float deltaTime, InputManager& input) {
    (void)deltaTime;

    for (auto& btn : buttons)
        btn->update(input);
}

void HomePageScreen::draw(sf::RenderWindow& window) {
    window.setView(view);
    for (auto& btn : buttons)
        btn->draw(window);
}
