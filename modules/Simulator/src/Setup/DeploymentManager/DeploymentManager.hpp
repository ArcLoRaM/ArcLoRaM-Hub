#pragma once

#include "../Factories/INodeFactory.hpp"
#include "../../Node/Node.hpp"
#include "../../Connectivity/Logger/Logger.hpp"
#include <memory>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <unordered_set>
#include "../../TDMA/TDMAEnums.hpp"

class DeploymentManager {
public:
    DeploymentManager(Logger& logger
                      
                      );

    // std::vector<std::shared_ptr<Node>> loadDeploymentFromFile(const std::string& filename);
    bool loadTopologyFromString(ScenarioType scenario, const std::string& topology);

    std::vector<std::shared_ptr<Node>> getParsedNodes();

private:
    Logger& logger;

   
    std::vector<std::shared_ptr<Node>> parsedNodes;


    // Mode-agnostic dispatcher
    void parseLine(const std::string& line,
                   INodeFactory& factory,
                   std::vector<std::shared_ptr<Node>>& nodes,
                   std::unordered_set<int>& nodeIds);


};
