#include "DeploymentManager.hpp"
#include "../Factories/FactorySelector/FactorySelector.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


DeploymentManager::DeploymentManager(Logger& logger
                                     )
    : logger(logger) {}


bool DeploymentManager::loadTopologyFromString(ScenarioType scenario, const std::string& topology)
{
    std::istringstream stream(topology);
    std::unordered_set<int> nodeIds;
    std::unique_ptr<INodeFactory> factory;

    try {
        factory = FactorySelector::getFactory(scenario, logger);
    } catch (const std::exception& ex) {
        logger.logCritical("Failed to get factory for scenario: " + std::string(ex.what()));
        return false;
    }

    std::string line;
    int lineNumber = 0;

    while (std::getline(stream, line)) {
        ++lineNumber;

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        try {
            parseLine(line, *factory, parsedNodes, nodeIds);
        } catch (const std::exception& ex) {
            logger.logCritical("Error parsing line " + std::to_string(lineNumber) + ": " + line);
            logger.logCritical("Reason: " + std::string(ex.what()));
            return false;
        }
    }

    return true;
}



std::vector<std::shared_ptr<Node>> DeploymentManager::getParsedNodes()
{
    return parsedNodes;
}


// std::istringstream iss(line);
    // std::string token;
    // iss >> token;

    // if (token != "NODE") {
    //     throw std::invalid_argument("Invalid line: must start with NODE.");
    // }

    // int id, x, y;
    // std::string type;
    // iss >> id >> type >> x >> y;

    // if (nodeIds.count(id)) {
    //     throw std::invalid_argument("Duplicate node ID detected: " + std::to_string(id));
    // }
    // nodeIds.insert(id);

    // if (type == "C3") {
    //     nodes.push_back(factory.createC3Node(id, {x, y}));
    // }
    // else if (type == "C2") {
    //     int nextHop = -1, hopCount = -1;
    //     std::string param;
    //     while (iss >> param) {
    //         auto delimiterPos = param.find('=');
    //         if (delimiterPos == std::string::npos) {
    //             throw std::invalid_argument("Invalid parameter format: " + param);
    //         }
    //         auto key = param.substr(0, delimiterPos);
    //         auto value = std::stoi(param.substr(delimiterPos + 1));

    //         if (key == "nextHop") nextHop = value;
    //         else if (key == "hopCount") hopCount = value;
    //         else {
    //             std::cerr << "Warning: Unknown parameter '" << key << "' for C2 node." << std::endl;
    //         }
    //     }

    //     if (nextHop == -1 || hopCount == -1) {
    //         throw std::invalid_argument("C2 node missing required parameters: nextHop and hopCount.");
    //     }

    //     nodes.push_back(factory.createC2Node(id, {x, y}, nextHop, hopCount));
    // }
    // else {
    //     throw std::invalid_argument("Unknown or unsupported node type for RRC_Uplink: " + type);
    // }


void DeploymentManager::parseLine(const std::string& line,
                                  INodeFactory& factory,
                                  std::vector<std::shared_ptr<Node>>& nodes,
                                  std::unordered_set<int>& nodeIds
                                  )
{
    

std::istringstream iss(line);
    std::string token;
    iss >> token;

    if (token != "NODE") {
        logger.logCritical("Invalid line: must start with NODE.");
        throw std::invalid_argument("Invalid line: must start with NODE.");
    }

    int id, x, y;
    std::string type;
    iss >> id >> type >> x >> y;

    if (nodeIds.count(id)) {
        logger.logCritical("Duplicate node ID detected: " + std::to_string(id));
        throw std::invalid_argument("Duplicate node ID detected: " + std::to_string(id));
    }
    nodeIds.insert(id);

    if (type == "C3") {
        nodes.push_back(factory.createC3Node(id, {x, y}));
    }
    else if (type == "C2") {
        std::optional<int> nextHop = std::nullopt;
        std::optional<int> hopCount = std::nullopt;
        
        std::string param;
        while (iss >> param) {
            auto delimiterPos = param.find('=');
            if (delimiterPos == std::string::npos || delimiterPos == param.size() - 1) {
                logger.logCritical("Invalid parameter format: " + param);
                throw std::invalid_argument("Invalid parameter format: " + param);
            }

            auto key = param.substr(0, delimiterPos);
            auto valueStr = param.substr(delimiterPos + 1);

            int value;
            try {
                value = std::stoi(valueStr);
            } catch (const std::exception& e) {
                logger.logCritical("Invalid integer value in parameter: " + param);
                throw;
            }

            
            if (key == "nextHop") nextHop = value;
            else if (key == "hopCount") hopCount = value;
            else {

                logger.logWarning("Unknown parameter '" + key + "' for C2 node.");
            }
        }

        nodes.push_back(factory.createC2Node(id, {x, y}, nextHop, hopCount));
    }
    else if (type == "C1") {
        // nodes.push_back(factory.createC1Node(id, {x, y}));
        logger.logCritical("C1 nodes are not supported yet");
        throw std::invalid_argument("C1 nodes are not supported yet");
    }
    else {
        logger.logCritical("Unknown node type: " + type);
        throw std::invalid_argument("Unknown node type: " + type);
    }
}


