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




bool TopologyConfigIO::write(const std::string& path,
    const std::unordered_map<int, std::unique_ptr<Device>>& nodes,
    const std::unordered_map<int, std::unordered_set<int>>& routings,
    TopologyMode mode)
{
    bool hasC3=false;

    std::ofstream outFile(path);
    if (!outFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    std::string modeStr=std::string(magic_enum::enum_name(mode));
    if (modeStr.empty()) {
        std::cerr << "Invalid topology mode: " << modeStr << "\n";
        return false;
    }
    outFile << "MODE " << modeStr << "\n";

    for (const auto& [id, device] : nodes) {
        const auto cls = device->getClass();
        const auto pos = device->getCenteredPosition();

        if (cls == DeviceClass::C3) hasC3=true;

        outFile << "NODE " << id << " "
                << std::string(magic_enum::enum_name(cls)) << " "
                << static_cast<int>(pos.x) << " "
                << static_cast<int>(pos.y);
        // Skip C3 routing logic
        if (mode == TopologyMode::RRC_Uplink && cls != DeviceClass::C3) {
            auto routingInfo = TopologyConfigIO::computeRoutingInfo(id, routings, nodes);
            if (!routingInfo) {
                std::cerr << "RRC_Uplink Mode Requirement Error: Unable to compute routing info for node " << id << "\n";
                return false;
            }
            auto [nextHop, hopCount] = *routingInfo;
            outFile << " nextHop=" << nextHop << " hopCount=" << hopCount;
        }
        outFile << "\n";
    }
    
    outFile.close();
    std::cout << "Topology configuration written to " << path << "\n";

    return hasC3; // Return true if at least one C3 was found
}



bool TopologyConfigIO::read(const std::string& path, TopologyEditorState& state) {
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    state.resetState();

    std::string line;
    TopologyMode mode;
    std::unordered_map<int, int> nextHopMap;
    bool foundMode = false;

    auto trim = [](std::string s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        return s;
    };

    while (std::getline(inFile, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "MODE") {
            std::string modeStr;
            iss >> modeStr;
            auto parsed = magic_enum::enum_cast<TopologyMode>(modeStr);
            if (!parsed) {
                std::cerr << "Invalid topology mode: " << modeStr << "\n";
                return false;
            }
            mode = *parsed;
            state.setTopologyMode(mode);
            foundMode = true;
        }

        else if (keyword == "NODE") {
            int id, x, y;
            std::string classStr;
            if (!(iss >> id >> classStr >> x >> y)) {
                std::cerr << "Invalid NODE entry: " << line << "\n";
                return false;
            }

            auto parsedClass = magic_enum::enum_cast<DeviceClass>(classStr);
            if (!parsedClass) {
                std::cerr << "Invalid device class: " << classStr << "\n";
                return false;
            }

            auto device = std::make_unique<Device>(id, *parsedClass, sf::Vector2f{float(x), float(y)}, 100.f);
            state.nodes[id] = std::move(device);
            state.nodeCounter = std::max(state.nodeCounter, id);

            std::string token;
            while (iss >> token) {
                if (token.rfind("nextHop=", 0) == 0) {
                    int nextHop = std::stoi(token.substr(8));
                    nextHopMap[id] = nextHop;
                }
            }
        }

        else {
            std::cerr << "Unknown directive: " << keyword << "\n";
        }
    }

    if (mode == TopologyMode::RRC_Uplink) {
        for (const auto& [from, to] : nextHopMap) {
            if (!state.nodes.count(from) || !state.nodes.count(to)) {
                std::cerr << "Invalid nextHop reference: " << from << " -> " << to << "\n";
                return false;
            }
            state.routings[from].insert(to);
        }
    }

    return foundMode;
}

