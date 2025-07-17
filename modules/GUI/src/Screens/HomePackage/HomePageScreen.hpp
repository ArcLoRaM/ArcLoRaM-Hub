#pragma once
#include "../../Shared/InputManager/InputManager.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../Screen.hpp"
#include "HomePageState.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"


class HomePageScreen : public Screen {
public:
    HomePageScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui);

    void handleEvent(InputManager& input) override;
    void update(float deltaTime, InputManager& input) override;
    void draw(sf::RenderWindow& window) override;
    void setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) override;

private:
};