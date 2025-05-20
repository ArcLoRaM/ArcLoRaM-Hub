#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>
#include <SFML/System/Vector2.hpp>

enum class EditorMode {
    Idle,
    AddingC1Node,
    AddingC2Node,
    AddingC3Node,
    AddingLink,
    CuttingNode,
    CuttingLink,
    MovingNode
};

enum class DeviceClass {
    C1,
    C2,
    C3
};

enum class TopologyMode {
    RRC_Uplink,
    RRC_Downlink,
    RRC_Beacon,
    ENC_Downlink,
    ENC_Uplink,
    ENC_Beacon
};


//this struct is going to be used to make the config file.
struct TopologyNode {
    int id;
    DeviceClass deviceClass;
    sf::Vector2f position;
    std::optional<int> nextHop;  // Only for RRC_Uplink and C2
    std::optional<int> hopCount; // Only for RRC_Uplink and C2
};

class TopologyEditorState {
public:
    void setTopologyMode(TopologyMode newMode);
    void setEditorMode(EditorMode newMode);
    EditorMode getEditorMode() const;
    // TopologyMode getMode() const;

    bool addNode(int id, DeviceClass cls, sf::Vector2f position);
    bool changeNodePosition(int id, sf::Vector2f newPosition);
    bool removeNode(int id);
    bool addLink(int id1, int id2);
    bool removeLink(int id1, int id2);

    std::optional<TopologyNode> getNode(int id) const;
    const std::unordered_map<int, TopologyNode>& getNodes() const;

    void clear();

private:

    TopologyMode currentMode{TopologyMode::RRC_Uplink};
    EditorMode currentEditorMode{EditorMode::Idle};
    std::unordered_map<int, TopologyNode> nodes;


};
