#pragma once
#include "../../Shared/InputManager/InputManager.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../../UI/Button/Button.hpp"
#include "../Screen.hpp"
#include "HomePageState.hpp"

class HomePageScreen : public Screen  {


    std::vector<std::unique_ptr<Button>> buttons;
    sf::View view;
    HomePageState state; // State of the home page

public:
    HomePageScreen();
    void handleEvent(InputManager& input);
    void update(InputManager& input);
    void draw(sf::RenderWindow& window);
};
