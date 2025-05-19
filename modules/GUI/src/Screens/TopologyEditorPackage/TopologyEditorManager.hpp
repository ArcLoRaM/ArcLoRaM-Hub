#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include "TopologyEditorState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include "../../UI/Dropdown/Dropdown.hpp"
#include "../../UI/Button/Button.hpp"

class TopologyEditorManager {
public:
    TopologyEditorManager(TopologyEditorState& state);

    void handleInput(const InputManager& input);
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

private:

    std::unordered_map<int, std::unique_ptr<Device>> visualNodes;

    std::optional<int> selectedNodeId;
    int nodeCounter = 0;

    void addVisualNode(int id, DeviceClass cls, sf::Vector2f position);
    std::optional<int> getNodeAtPosition(sf::Vector2f position);

    Dropdown<TopologyMode> modeDropdown;
    TopologyEditorState& state;
    sf::RectangleShape topologyBounds;


    std::unique_ptr<Button> saveButton;
    std::unique_ptr<Button> addLinkButton;  
    std::unique_ptr<Button> addNodeC1Button;
    std::unique_ptr<Button> addNodeC2Button;
    std::unique_ptr<Button> addNodeC3Button;

    
};

