#include "TopologyEditorState.hpp"
#include <iostream>

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


std::unordered_map<int, std::unique_ptr<Device>>::iterator TopologyEditorState::removeNode(std::unordered_map<int, std::unique_ptr<Device>>::iterator it)
{
    if (it != nodes.end()) {
        return nodes.erase(it);  // safe erase, returns next valid iterator
    }
    return nodes.end();
}



void TopologyEditorState::addNode( DeviceClass cls, sf::Vector2f pos)
{
    auto battery = 100.0; // dummy default
    
    nodeCounter++;
    if (nodes.find(nodeCounter) != nodes.end()) {
        std::cerr << "********Node with this ID already exists.********\n";
    }

    nodes[nodeCounter] = std::make_unique<Device>(nodeCounter, cls, pos, battery);

}


bool TopologyEditorState::addRouting()
{
    if(!link.isComplete())
    {
        std::cout << "********Link is not complete.********\n";
        return false;
    }
    int id1 = link.nodes[0];
    int id2 = link.nodes[1];
    
    if(id1 == id2)
    {
        std::cout << "********Cannot connect a device to itself.********\n";
        return false;
    }
    // ID1 ---> ID2 , path towards ID2
    if (nodes.count(id1) && nodes.count(id2))
    {
        // Check if the devices are already connected
        if (routings[id1].count(id2) > 0)
        {
            std::cout << "********Devices are already connected.********\n";
            return false;
        }
            
        else   // Add the routing
        {
            routings[id1].insert(id2); // Only store id2 in id1's adjacency list
            return true;
        } 
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
        return false;
    }
}

void TopologyEditorState::removeSpecificRouting()
{
    int id1 = link.nodes[0];
    int id2 = link.nodes[1];

    if (nodes.count(id1) && nodes.count(id2)) {
        // Check if the devices are connected
        if (routings[id1].count(id2) > 0) {
            routings[id1].erase(id2); // Remove the routing
        } else {
            std::cout << "********Devices are not connected.********\n";
        }
    } else {
        std::cout << "********One or both devices do not exist.********\n";
    }
}

bool TopologyEditorState::removeRouting(std::optional<int> id1, std::optional<int> id2)
{
    // Invalid if neither are provided
    if (!id1 && !id2) {
        std::cerr << "Error: Must provide at least one ID.\n";
        return false;
    }

    if (id1 && id2) {
        if(*id1 == *id2)
        {
            std::cout << "********Cannot remove a link to itself.********\n";
            return false;
        }

        // Case 1: remove specific link from id1 to id2
        if (!nodes.count(*id1) || !nodes.count(*id2)) {
            std::cout << "********One or both devices do not exist.********\n";
            return false;
        }

        auto it = routings.find(*id1);
        if (it != routings.end() && it->second.erase(*id2) > 0) {
            return true;
        } else {
            std::cout << "********Devices are not connected.********\n";
            return false;
        }
    }

    // One of the two is set
    int targetId = id1 ? *id1 : *id2;

    if (!nodes.count(targetId)) {
        std::cout << "********Device " << targetId << " does not exist.********\n";
        return false;
    }

    bool removedAny = false;

    // Remove all outbound links
    if (routings.erase(targetId) > 0) {
        removedAny = true;
    }

    // Remove all inbound links
    for (auto& [from, targets] : routings) {
        if (targets.erase(targetId) > 0) {
            removedAny = true;
        }
    }

    if (!removedAny) {
        std::cout << "********No routings found involving device " << targetId << ".********\n";
    }

    return removedAny;
}