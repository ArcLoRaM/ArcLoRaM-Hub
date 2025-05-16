#pragma once
#include "../../Shared/InputManager/InputManager.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "../../UI/Button/Button.hpp"
#include "../Screen.hpp"
#include "HomePageState.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"


class HomePageScreen : public Screen {
public:
    HomePageScreen(std::vector<std::pair<std::string, ScreenAction>> actions);

    void handleEvent(InputManager& input) override;
    void update(float deltaTime, InputManager& input) override;
    void draw(sf::RenderWindow& window) override;

private:
    std::vector<std::unique_ptr<Button>> buttons;
    sf::View view;
};