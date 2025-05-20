#include "TopologyEditorState.hpp"

void TopologyEditorState::setTopologyMode(TopologyMode newMode)
{
    currentMode = newMode;
}

void TopologyEditorState::setEditorMode(EditorMode newMode)
{
    currentEditorMode = newMode;
}

EditorMode TopologyEditorState::getEditorMode() const
{
    return currentEditorMode;
}

bool TopologyEditorState::addNode(int id, DeviceClass cls, sf::Vector2f position)
{
    if (nodes.find(id) != nodes.end()) {
        return false; // Node with this ID already exists
    }

    nodes.emplace(id, TopologyNode{id, cls, position, std::nullopt, std::nullopt});
    return true;

}

bool TopologyEditorState::removeNode(int id)
{
    if (nodes.find(id) != nodes.end()) {
        nodes.erase(id);
        return true; // Node with this ID already exists
    }
    return false;
}
