#include "DeploymentManager.hpp"
#include "../Factories/FactorySelector/FactorySelector.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


DeploymentManager::DeploymentManager(Logger& logger
                                     )
    : logger(logger) {}


bool DeploymentManager::loadTopologyFromString(ScenarioType scenario, const std::string &topology)
{
    try {
        std::istringstream stream(topology);

       
        std::unordered_set<int> nodeIds;

        std::unique_ptr<INodeFactory> factory = FactorySelector::getFactory(scenario, logger);

        std::string line;
        bool modeParsed = false;

        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == '#') continue;

            parseLine(line, *factory, parsedNodes, nodeIds);
        }


        return true;
    } catch (const std::exception& ex) {
        logger.logSystem("Error parsing deployment from string");
        return false;
    }
}


std::vector<std::shared_ptr<Node>> DeploymentManager::getParsedNodes()
{
    return parsedNodes;
}



void DeploymentManager::parseLine(const std::string& line,
                                  INodeFactory& factory,
                                  std::vector<std::shared_ptr<Node>>& nodes,
                                  std::unordered_set<int>& nodeIds
                                  )
{


//   todo: make a parse Line versatile that instantiate everything depending on the node line,
//   check if other parameters such as distnace treshold are not included here 
//   think long term: what could be present in the config file that messes up with this?
//   are there going to be a lof of optional initialization variable for the nodes?
//   question: how can we make sure that an incorrect topology (ex: a missing nextId or hop count) is declared as incompatible with certain scenarios? ex: simonV1 -> start by RRC_Uplink, assumption is beacon phase happened before so there needs ot be these nextId and hop count __cpp_variable_templates 
    
//     should it be done in the GUI, here? somewhere else?;



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
        std::optional<int> nextHop = std::nullopt;
        std::optional<int> hopCount = std::nullopt;
        
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
                logger.logSystem("Warning: Unknown parameter '" + key + "' for C2 node.");
            }
        }

        nodes.push_back(factory.createC2Node(id, {x, y}, nextHop, hopCount));
    }
    else if (type == "C1") {
        // nodes.push_back(factory.createC1Node(id, {x, y}));
        throw std::invalid_argument("C1 nodes are not supported yet");
    }
    else {
        throw std::invalid_argument("Unknown node type: " + type);
    }
}


