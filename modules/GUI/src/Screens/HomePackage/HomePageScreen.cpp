#include "HomePageScreen.hpp"
#include "../../Shared/Config.hpp"
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../UI/UIFactory/UIFactory.hpp"


HomePageScreen::HomePageScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui)
    : Screen(gui)
{

    setupUI(actions);
 
}

void HomePageScreen::setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) {
    gui.removeAllWidgets(); // Clear any previous screen’s widgets

    updateTextSize(0.07f); // Set text size based on window height

    auto picture = tgui::Picture::create("assets/background.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);


const tgui::String buttonWidth = "45%";
const tgui::String buttonHeight = "10%";
const float spacingPercent = 3.f; // spacing between buttons in %

const std::size_t numButtons = actions.size();
const float totalHeightPercent = numButtons * 10.f + (numButtons - 1) * spacingPercent;
const float startYPercent = 50.f - (totalHeightPercent / 2.f);

for (std::size_t i = 0; i < numButtons; ++i) {
    const auto& [label, action] = actions[i];

    auto button = UIFactory::createButton(label, action);
    if (!button)
        throw std::runtime_error("Failed to create button: " + label);

    button->setSize(buttonWidth, buttonHeight);

    float y = startYPercent + i * (10.f + spacingPercent);
    button->setPosition("27.5%", tgui::String(std::to_string(y) + "%")); // center: (100% - 30%) / 2 = 35%

    gui.add(button);
}

}


void HomePageScreen::handleEvent(InputManager& input) {
    // Possible future use (navigation, etc.)
}




void HomePageScreen::update(float deltaTime, InputManager& input) {

}

void HomePageScreen::draw(sf::RenderWindow& window) {

}
