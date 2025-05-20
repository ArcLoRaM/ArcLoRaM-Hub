#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <memory>
#include <cmath>
#include "TopologyEditorState.hpp"
#include "../../Visualisation/Device/Device.hpp"
#include "../../UI/Dropdown/Dropdown.hpp"
#include "../../UI/Button/Button.hpp"


struct PendingLink {
    std::vector<int> nodes;

    void add(int id) {
        if (nodes.size() < 2) nodes.push_back(id);
    }

    void reset() { nodes.clear(); }

    bool isComplete() const { return nodes.size() == 2; }

    bool operator==(const PendingLink& other) const {
        return nodes == other.nodes;
    }
};




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


    std::optional<sf::Text> coordText;
    std::optional<int> selectedNodeId;
    int nodeCounter = 0;

    void addVisualNode(int id, DeviceClass cls, sf::Vector2f position);
    std::optional<int> getNodeAtPosition(sf::Vector2f position);

    Dropdown<TopologyMode> modeDropdown;
    TopologyEditorState& state;
    sf::RectangleShape topologyBounds;

    std::unique_ptr<Button> saveButton;
    std::unique_ptr<Button> addLinkButton;
    std::unique_ptr<Button> cutNodeButton;  
    std::unique_ptr<Button> cutLinkButton;
    std::unique_ptr<Button> addNodeC1Button;
    std::unique_ptr<Button> addNodeC2Button;
    std::unique_ptr<Button> addNodeC3Button;
    std::unique_ptr<Button> moveNodeButton;

    std::unordered_map<int, std::unique_ptr<Device>> visualNodes;


    //Routing: Todo: put the routings in the TopologyEditorState
    std::unordered_map<int, std::unordered_set<int>> routings; // Oriented Graph ! 2->4 is not equl to 4->2
    bool routingDisplayEnabled = false; // Flag to control routing display
    void drawRootings(sf::RenderWindow& window);
    bool addRouting(int id1, int id2);
    bool removeRouting(std::optional<int> id1, std::optional<int> id2);

    PendingLink link;
    

};



