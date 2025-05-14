#include "DeploymentManager.hpp"
#include "../Factories/FactorySelector/FactorySelector.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


DeploymentManager::DeploymentManager(Logger& logger,
                                     std::condition_variable& dispatchCv,
                                     std::mutex& dispatchCvMutex,
                                     uint64_t baseTime)
    : logger(logger), dispatchCv(dispatchCv), dispatchCvMutex(dispatchCvMutex), baseTime(baseTime) {}

std::vector<std::shared_ptr<Node>> DeploymentManager::loadDeploymentFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open deployment config file: " + filename);
    }

    std::vector<std::shared_ptr<Node>> nodes;
    std::unordered_set<int> nodeIds;
    std::unique_ptr<INodeFactory> factory = nullptr;
    common::CommunicationMode mode;

    std::string line;
    bool modeParsed = false;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (!modeParsed) {
            mode = parseModeLine(line);
            factory = FactorySelector::getFactory(mode, logger, dispatchCv, dispatchCvMutex, baseTime);
            modeParsed = true;
            continue;
        }

        parseLine(line, *factory, nodes, nodeIds, mode);
    }

    if (!modeParsed) {
        throw std::runtime_error("Deployment file missing mandatory MODE declaration.");
    }

    return nodes;
}

common::CommunicationMode DeploymentManager::parseModeLine(const std::string& line)
{
    std::istringstream iss(line);
    std::string token, modeString;
    iss >> token >> modeString;
    if (token != "MODE") {
        throw std::invalid_argument("Deployment file must start with 'MODE <ModeName>'.");
    }

    if (modeString == "RRC_Uplink") return common::CommunicationMode::RRC_Uplink;
    if (modeString == "RRC_Downlink") return common::CommunicationMode::RRC_Downlink;
    if (modeString == "RRC_Beacon") return common::CommunicationMode::RRC_Beacon;
    if (modeString == "ENC_Uplink") return common::CommunicationMode::ENC_Uplink;
    if (modeString == "ENC_Downlink") return common::CommunicationMode::ENC_Downlink;
    if (modeString == "ENC_Beacon") return common::CommunicationMode::ENC_Beacon;

    throw std::invalid_argument("Unknown MODE specified: " + modeString);
}

void DeploymentManager::parseLine(const std::string& line,
                                  INodeFactory& factory,
                                  std::vector<std::shared_ptr<Node>>& nodes,
                                  std::unordered_set<int>& nodeIds,
                                  common::CommunicationMode mode)
{
    switch (mode) {
        case common::CommunicationMode::RRC_Uplink:
            parseLineRrcUplink(line, factory, nodes, nodeIds);
            break;
        case common::CommunicationMode::RRC_Downlink:
            parseLineRrcDownlink(line, factory, nodes, nodeIds);
            break;
        case common::CommunicationMode::ENC_Uplink:
            parseLineEncUplink(line, factory, nodes, nodeIds);
            break;
        // etc...
        default:
            throw std::invalid_argument("Unsupported mode in line parsing.");
    }
}

void DeploymentManager::parseLineRrcUplink(const std::string& line, INodeFactory& factory,
                                           std::vector<std::shared_ptr<Node>>& nodes,
                                           std::unordered_set<int>& nodeIds)
{
    std::istringstream iss(line);
    std::string token;
    iss >> token;

    if (token != "NODE") {
        throw std::invalid_argument("Invalid line: must start with NODE.");
    }

    int id, x, y;
    std::string type;
    iss >> id >> type >> x >> y;

    if (nodeIds.count(id)) {
        throw std::invalid_argument("Duplicate node ID detected: " + std::to_string(id));
    }
    nodeIds.insert(id);

    if (type == "C3") {
        nodes.push_back(factory.createC3Node(id, {x, y}));
    }
    else if (type == "C2") {
        int nextHop = -1, hopCount = -1;
        std::string param;
        while (iss >> param) {
            auto delimiterPos = param.find('=');
            if (delimiterPos == std::string::npos) {
                throw std::invalid_argument("Invalid parameter format: " + param);
            }
            auto key = param.substr(0, delimiterPos);
            auto value = std::stoi(param.substr(delimiterPos + 1));

            if (key == "nextHop") nextHop = value;
            else if (key == "hopCount") hopCount = value;
            else {
                std::cerr << "Warning: Unknown parameter '" << key << "' for C2 node." << std::endl;
            }
        }

        if (nextHop == -1 || hopCount == -1) {
            throw std::invalid_argument("C2 node missing required parameters: nextHop and hopCount.");
        }

        nodes.push_back(factory.createC2Node(id, {x, y}, nextHop, hopCount));
    }
    else {
        throw std::invalid_argument("Unknown or unsupported node type for RRC_Uplink: " + type);
    }
}

void DeploymentManager::parseLineRrcDownlink(const std::string& line, INodeFactory& factory,
                                             std::vector<std::shared_ptr<Node>>& nodes,
                                             std::unordered_set<int>& nodeIds)
{
    // Placeholder for RRC_Downlink-specific parsing (to be implemented)
}

void DeploymentManager::parseLineEncUplink(const std::string& line, INodeFactory& factory,
                                           std::vector<std::shared_ptr<Node>>& nodes,
                                           std::unordered_set<int>& nodeIds)
{
    // Placeholder for ENC_Uplink-specific parsing (to be implemented)
}
