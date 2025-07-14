#pragma once

#include "../Screen.hpp"
#include "../../Ui/Button/Button.hpp"
#include <SFML/Graphics.hpp>
#include <filesystem>
#include <memory>
#include <vector>
#include "TopologyEditorManager.hpp"
#include "TopologyEditorState.hpp"

class TopologyEditorScreen : public Screen {

public:
    // Constructor for new topology (empty editor)
    explicit TopologyEditorScreen(std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui);

    // Constructor for editing existing topology from file
    TopologyEditorScreen(const std::string& topologyFilePath, std::vector<std::pair<std::string, ScreenAction>> actions,tgui::Gui& gui);
    void setupUI(std::vector<std::pair<std::string, ScreenAction>> actions) override;

    // Overridden Screen methods
    void handleEvent(InputManager& input) override;
    void update(float deltaTime, InputManager& input) override;
    void draw(sf::RenderWindow& window) override;

private:

    sf::View editorView;    
    TopologyEditorState editorState;
    TopologyEditorManager editorManager;
    
    void onResize() override;
};
