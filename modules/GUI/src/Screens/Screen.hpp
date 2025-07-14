#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "../Shared/InputManager/InputManager.hpp"
#include <functional>
#include <TGUI/TGUI.hpp>  // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>


using ScreenAction = std::function<void()>;
class Screen {
public:
    virtual ~Screen() = default;
    virtual void setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) = 0;

    //todo:should change the name to handleInput
    virtual void handleEvent(InputManager& input) = 0;
    virtual void update(float deltaTime,InputManager &input) = 0;

    //since raw SFML and TGUI cohabit, we keep this custom draw function
    virtual void draw(sf::RenderWindow& window) = 0;

   explicit Screen(tgui::Gui& guiRef): gui(guiRef) {};

    //make a list of key-value pairs to store the lambdas:
    // using ScreenChangeCallback = std::function<void(std::unique_ptr<Screen>)>;
        
    virtual void onResize();

    protected:
    tgui::Gui& gui;
    void updateTextSize(float percentage=0.07f); // ✅ shared for all screens

};
