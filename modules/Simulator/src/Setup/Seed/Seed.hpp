#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../../Node/Node.hpp"
#include "../../Connectivity/Logger/Logger.hpp"
#include <utility> // For std::pair
#include "../../Node/C3/C3_Node.hpp"
#include "../../Node/C2/C2_Node.hpp"
#include "../../Node/C1/C1_Node.hpp"

#include "../../TDMA/TDMAEnums.hpp"


class Seed {

public:

    Seed(std::string topology,ScenarioType scenario,Logger& logger) : topology(topology), logger(logger), scenario(scenario) {

        if(!instantiateTopology(topology))
        {
            throw std::runtime_error("Failed to instantiate topology");
        }
    }

    std::vector<std::shared_ptr<Node>> transferOwnership();
  
    // Get the blueprint created by the factory (for logging/debugging if needed)
    std::shared_ptr<IScheduleBlueprint> getBlueprint() const { return blueprint; }
private:
    Logger& logger;
    std::string topology;
    ScenarioType scenario;
    std::vector<std::shared_ptr<Node>> listNode;
    bool instantiateTopology(std::string topology);
    std::shared_ptr<IScheduleBlueprint> blueprint;
};
