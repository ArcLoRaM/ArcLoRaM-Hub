#include "TopologyConfigIO.hpp"
#include "magic_enum.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>


// Helper function to compute nextHop and hopCount for a node toward any C3
std::optional<std::pair<int, int>> TopologyConfigIO::computeRoutingInfo(
    int startId,
    const std::unordered_map<int, std::unordered_set<int>>& routings,
    const std::unordered_map<int, std::unique_ptr<Device>>& nodes)
{
    int current = startId;
    int hopCount = 0;
    int nextHop = 0;
    std::unordered_set<int> visited;

    while (true) {
        if (visited.count(current)) {
            std::cerr << "Cycle detected in routing path starting from node " << startId << ".\n";
            return std::nullopt;
        }

        visited.insert(current);

        auto it = routings.find(current);

        if (it == routings.end() || it->second.empty()) {
            const auto& dev = nodes.at(current);
            if (dev->getClass() == DeviceClass::C3) {
                return std::make_pair(nextHop, hopCount);
            } else {
                std::cerr << "Dead-end routing: Node " << current << " is not a C3 and has no outgoing route.\n";
                return std::nullopt;
            }
        }

        if (it->second.size() != 1) {
            std::cerr << "Ambiguous routing: Node " << current << " has " << it->second.size() << " outgoing routes.\n";
            return std::nullopt;
        }

        int next = *it->second.begin();
        if (hopCount == 0) {
            nextHop = next;
        }

        hopCount++;
        current = next;
    }
}




void TopologyConfigIO::write(const std::string& path,
    const std::unordered_map<int, std::unique_ptr<Device>>& nodes,
    const std::unordered_map<int, std::unordered_set<int>>& routings,
    TopologyMode mode)
{
    std::ofstream outFile(path);
    if (!outFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return;
    }

    outFile << "MODE " << std::string(magic_enum::enum_name(mode)) << "\n";

    for (const auto& [id, device] : nodes) {
        const auto cls = device->getClass();
        const auto pos = device->getCenteredPosition();

        outFile << "NODE " << id << " "
                << std::string(magic_enum::enum_name(cls)) << " "
                << static_cast<int>(pos.x) << " "
                << static_cast<int>(pos.y);

        // Skip C3 routing logic
        if (mode == TopologyMode::RRC_Uplink && cls != DeviceClass::C3) {
            auto routingInfo = TopologyConfigIO::computeRoutingInfo(id, routings, nodes);
            if (!routingInfo) {
                std::cerr << "Error: Unable to compute routing info for node " << id << "\n";
                continue; // Skip this node on error
            }
            auto [nextHop, hopCount] = *routingInfo;
            outFile << " nextHop=" << nextHop << " hopCount=" << hopCount;
        }
        outFile << "\n";
    }
    outFile.close();
    std::cout << "Topology configuration written to " << path << "\n";
}



bool TopologyConfigIO::read(const std::string& path, TopologyEditorState& state) {
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    state.resetState();

    std::string line;
    TopologyMode mode = TopologyMode::RRC_Uplink; // Default fallback
    std::unordered_map<int, int> nextHopMap;

    while (std::getline(inFile, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);

        // Parse topology mode
        if (line.rfind("topologyMode=", 0) == 0) {
            std::string modeStr = line.substr(14);
            auto parsedMode = magic_enum::enum_cast<TopologyMode>(modeStr);
            if (!parsedMode) {
                std::cerr << "Invalid topology mode: " << modeStr << "\n";
                return false;
            }
            mode = *parsedMode;
            state.setTopologyMode(mode);
            continue;
        }

        // Parse NODE entries
        if (line.rfind("NODE", 0) == 0) {
            std::string token, classStr;
            int id, x, y;
            int nextHop = -1;

            iss >> token >> id >> classStr >> x >> y;

            auto parsedClass = magic_enum::enum_cast<DeviceClass>(classStr);
            if (!parsedClass) {
                std::cerr << "Invalid device class: " << classStr << "\n";
                return false;
            }

            sf::Vector2f pos(static_cast<float>(x), static_cast<float>(y));
            auto device = std::make_unique<Device>(id, *parsedClass, pos, 100.0);

            state.nodes[id] = std::move(device);
            state.nodeCounter = std::max(state.nodeCounter, id);

            std::string optionalToken;
            while (iss >> optionalToken) {
                if (optionalToken.rfind("nextHop=", 0) == 0) {
                    nextHop = std::stoi(optionalToken.substr(8));
                    nextHopMap[id] = nextHop;
                }
                // hopCount is ignored — derived info
            }
        }
    }

    // Construct routing links if needed
    if (mode == TopologyMode::RRC_Uplink) {
        for (const auto& [from, to] : nextHopMap) {
            if (!state.nodes.count(from) || !state.nodes.count(to)) {
                std::cerr << "Invalid nextHop reference: " << from << " -> " << to << "\n";
                return false;
            }
            state.routings[from].insert(to);
        }
    }

    return true;
}

