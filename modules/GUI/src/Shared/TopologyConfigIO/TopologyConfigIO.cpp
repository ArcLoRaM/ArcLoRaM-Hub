#include "TopologyConfigIO.hpp"
#include "magic_enum.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

//TODO: when we will add C1, we need to adapt the write funciton
// Helper function to compute nextHop and hopCount for a node toward any C3
std::optional<std::pair<int, int>> TopologyConfigIO::computeRoutingInfo(
    int startId,
    const std::unordered_map<int, std::unordered_set<int>>& routings,
    const std::unordered_map<int, std::unique_ptr<Device>>& nodes)
{
    auto it = routings.find(startId);
    
    // Starting node has no routing
    if (it == routings.end() || it->second.empty()) {
        return std::make_pair(-1, 0);
    }
    
    int current = startId;
    int hopCount = 0;
    int nextHop = 0;
    std::unordered_set<int> visited;

    while (true) {
        // Cycle detection
        if (visited.count(current)) {
            std::cerr << "Cycle detected in routing path starting from node " << startId << ".\n";
            return std::nullopt;
        }
        visited.insert(current);
        
        it = routings.find(current);
        
        // Ambiguous routing
        if (it != routings.end() && it->second.size() != 1) {
            std::cerr << "Ambiguous routing: Node " << current << " has " << it->second.size() << " outgoing routes.\n";
            return std::nullopt;
        }
        
        const auto& dev = nodes.at(current);
        
        // Reached C3
        if (dev->getClass() == DeviceClass::C3) {
            if (it != routings.end() && !it->second.empty()) {
                std::cerr << "Error: C3 node " << current << " has routing configured.\n";
                return std::nullopt;
            }
            return std::make_pair(nextHop, hopCount);
        }
        
        // No routing on intermediate/current node - return what we have so far
        if (it == routings.end() || it->second.empty()) {
            return std::make_pair(nextHop, hopCount);
        }
        
        // Follow the route
        int next = *it->second.begin();
        if (hopCount == 0) {
            nextHop = next;
        }
        hopCount++;
        current = next;
    }
}
bool TopologyConfigIO::write(const std::string &path, const std::unordered_map<int, std::unique_ptr<Device>> &nodes, const std::unordered_map<int, std::unordered_set<int>> &routings)
{
    // PHASE 1: VALIDATION
    bool hasC3 = false;
    
    for (const auto& [id, device] : nodes) {
        const auto cls = device->getClass();
        
        if (cls == DeviceClass::C3) {
            hasC3 = true;
        } else {
            // Validate routing for non-C3 nodes
            auto routingInfo = TopologyConfigIO::computeRoutingInfo(id, routings, nodes);
            if (!routingInfo) {
                std::cerr << "RRC_Uplink Mode Requirement Error: Unable to compute routing info for node " << id << "\n";
                return false;
            }
        }
    }
    
    if (!hasC3) {
        std::cerr << "Error: No C3 node found in topology.\n";
        return false;
    }
    
    // PHASE 2: WRITE (only if validation passed)
    std::ofstream outFile(path);
    if (!outFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    for (const auto& [id, device] : nodes) {
        const auto cls = device->getClass();
        const auto pos = device->getCenteredPosition();

        outFile << "NODE " << id << " "
                << std::string(magic_enum::enum_name(cls)) << " "
                << static_cast<int>(pos.x) << " "
                << static_cast<int>(pos.y);
        
        if (cls != DeviceClass::C3) {
            auto routingInfo = TopologyConfigIO::computeRoutingInfo(id, routings, nodes);
            // We already validated, so this should always succeed
            auto [nextHop, hopCount] = *routingInfo;
            
            if (nextHop != -1) {
                outFile << " nextHop=" << nextHop << " hopCount=" << hopCount;
            }
        }
        outFile << "\n";
    }
    
    outFile.close();
    std::cout << "Topology configuration written to " << path << "\n";

    return true;
}


bool TopologyConfigIO::read(const std::string &path, TopologyEditorState &state)
{
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    state.resetState();
    return validateConfigFile(inFile, nullptr, &state);
}


bool TopologyConfigIO::readTopologyConfig(const std::string& path, SimulationConfiguration& state) {
    std::ifstream inFile(path);
    if (!inFile) {
        std::cerr << "Failed to open config file: " << path << "\n";
        return false;
    }

    state.resetState();
    state.setDistanceThreshold(1000.0f);

    std::ostringstream topologyBuffer;
    bool foundTDMA = false;

    auto trim = [](std::string s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        return s;
    };

    std::string line;
    while (std::getline(inFile, line)) {
        std::string originalLine = line;
        line = trim(line);

        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        
            // Treat all non-MODE lines as topology text
            topologyBuffer << originalLine << "\n";
        }

    state.setTopologyLines(topologyBuffer.str());
    return true;
}

bool TopologyConfigIO::validateConfigFile(std::istream& in, std::string* outText, TopologyEditorState* outState) {
    std::string line;
    std::unordered_map<int, int> nextHopMap;


    std::unordered_map<int, std::unique_ptr<Device>>* nodes = nullptr;
    std::unordered_map<int, std::unordered_set<int>>* routings = nullptr;

    if (outState) {
        nodes = &outState->nodes;
        routings = &outState->routings;
    }

    auto trim = [](std::string s) {
        s.erase(0, s.find_first_not_of(" \t\r\n"));
        s.erase(s.find_last_not_of(" \t\r\n") + 1);
        return s;
    };

    std::ostringstream buffer;

    while (std::getline(in, line)) {
        std::string originalLine = line;
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        if (outText) buffer << originalLine << "\n";

        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "NODE") {
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

            if (nodes) {
                auto device = std::make_unique<Device>(id, *parsedClass, sf::Vector2f{float(x), float(y)});
                (*nodes)[id] = std::move(device);
                outState->nodeCounter = std::max(outState->nodeCounter, id);
            }

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
            return false;
        }
    }

    if ( routings) {
        for (const auto& [from, to] : nextHopMap) {
            if (!nodes->count(from) || !nodes->count(to)) {
                std::cerr << "Invalid nextHop reference: " << from << " -> " << to << "\n";
                return false;
            }
            (*routings)[from].insert(to);
        }
    }

    if (outText) *outText = buffer.str();

    return true;
}


