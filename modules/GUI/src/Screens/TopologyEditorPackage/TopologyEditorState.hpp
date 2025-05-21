#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include "../../Visualisation/Device/Device.hpp"
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


enum class TopologyMode {
    RRC_Uplink,
    RRC_Downlink,
    RRC_Beacon,
    ENC_Downlink,
    ENC_Uplink,
    ENC_Beacon
};

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

//this struct is going to be used to make the config file.
// struct TopologyNode {
//     int id;
//     DeviceClass deviceClass;
//     sf::Vector2f position;
//     std::optional<int> nextHop;  // Only for RRC_Uplink and C2
//     std::optional<int> hopCount; // Only for RRC_Uplink and C2
// };

class TopologyEditorState {
public:
    void setTopologyMode(TopologyMode newMode);

    // TopologyMode getMode() const;

    std::unordered_map<int, std::unique_ptr<Device>>::iterator removeNode(std::unordered_map<int, std::unique_ptr<Device>>::iterator it);

    const std::unordered_map<int, std::unique_ptr<Device>>& getNodes() const{
        return nodes;
    }

    const std::unordered_map<int, std::unordered_set<int>>& getRoutings() const{
        return routings;
    }
    
    void resetLink() {
        link.reset();
    }
    void addLink(int id)
    {
        link.add(id);

    }
    bool isLinkComplete() const {
        return link.isComplete();
    }
    void clear();

    
    void setEditorMode(EditorMode newMode);

    EditorMode getEditorMode() const;
    void addNode( DeviceClass cls, sf::Vector2f position);
    bool addRouting();
    bool removeRouting(std::optional<int> id1, std::optional<int> id2);
void removeSpecificRouting();

    
private:
    
    EditorMode currentEditorMode{EditorMode::Idle};

    TopologyMode currentMode{TopologyMode::RRC_Uplink};


    std::unordered_map<int, std::unique_ptr<Device>> nodes;



    //Routing: 
    std::unordered_map<int, std::unordered_set<int>> routings; // Oriented Graph ! 2->4 is not equl to 4->2
    bool routingDisplayEnabled = false; // Flag to control routing display

    PendingLink link;
    int nodeCounter = 0;


};

