#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "TopologyEditorState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include "../../UI/Dropdown/Dropdown.hpp"
#include "../../UI/Button/Button.hpp"
#include "../../Visualisation/Animations/BroadcastAnimation/BroadcastAnimation.hpp"
#include "../../UI/TypeableInput/TypeableInput.hpp" 
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>

class TopologyEditorManager {

public:
    TopologyEditorManager(TopologyEditorState& state,tgui::Gui& gui);
    void setupUI(tgui::Gui& gui,sf::View& editorView);
    void handleInput( InputManager& input);
    void update(float deltaTime);
    void draw(sf::RenderWindow& window,sf::View& editorView);
    void setSelectedNode(std::optional<int> nodeId);
    std::optional<int> getSelectedNode() const;

    sf::RectangleShape convertRectangleToTopologyView(
    const sf::RenderWindow& window,
    const sf::View& topologyView,
    const sf::RectangleShape& screenRect
);


bool isBoundsFullyInsideRect(
    const sf::Vector2f& position,  // top-left corner
    const sf::Vector2f& size,
    const sf::FloatRect& rect
);

    //to do: make getter and setView
    tgui::CanvasSFML::Ptr canvas;



private:

    std::optional<int> selectedNodeId;
 
std::map<EditorMode, tgui::ToggleButton::Ptr> toolButtons;
    TopologyEditorState& state;

    tgui::Gui& gui;

    sf::RectangleShape topologyBounds;
    //UI
    tgui::Label::Ptr coordLabel;

    //broadcast animations
    void startBroadcast(const sf::Vector2f& startPosition, float duration);
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcastAnimations; 
    void drawRootings(tgui::CanvasSFML::Ptr canvas);




    
};



