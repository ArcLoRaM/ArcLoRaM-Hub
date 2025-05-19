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
    explicit TopologyEditorScreen(ScreenAction backAction);

    // Constructor for editing existing topology from file
    TopologyEditorScreen(const std::filesystem::path& topologyFilePath, ScreenAction backAction);

    // Overridden Screen methods
    void handleEvent(InputManager& input) override;
    void update(float deltaTime, InputManager& input) override;
    void draw(sf::RenderWindow& window) override;

private:
    // Common initialization code (DRY principle)
    void initializeUI(ScreenAction backAction);
    // void loadTopologyFromFile(const std::filesystem::path& filePath);

    sf::View editorView;
    std::unique_ptr<Button> backButton;
    
  
    TopologyEditorState editorState;
    TopologyEditorManager editorManager;



};
