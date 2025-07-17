#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "TopologyEditorState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include "../../Visualisation/Animations/BroadcastAnimation/BroadcastAnimation.hpp"
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>

class TopologyEditorManager
{

public:
    TopologyEditorManager(TopologyEditorState &state, tgui::Gui &gui);
    void setupUI(sf::View &editorView);
    void handleInput(InputManager &input);
    void update(float deltaTime);
    void draw(sf::RenderWindow &window, sf::View &editorView);
    void setSelectedNode(std::optional<int> nodeId);
    std::optional<int> getSelectedNode() const;

    // to do: make getter and setView
    tgui::CanvasSFML::Ptr canvas;
    tgui::ComboBox::Ptr modeDropdown;

private:
    std::optional<int> selectedNodeId;

    std::map<EditorMode, tgui::ToggleButton::Ptr> toolButtons;
    TopologyEditorState &state;

    tgui::Gui &gui;

    // UI
    tgui::Label::Ptr coordLabel;
    void resetToggleButtons();

    // broadcast animations
    void startBroadcast(const sf::Vector2f &startPosition, float duration);
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations;
    void drawRootings(tgui::CanvasSFML::Ptr canvas);
};
