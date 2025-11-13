
#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include "../../Objects/Device/Device.hpp"


//Scenario types should be in ascending order
//ex; sce1=1, sce2=2, etc or will cause mismatch in scenario selection
enum class ScenarioType {
    SimonV1=0
};



class SimulationConfiguration {

/*
Holds the state of the parsed topology, from the configuration file.
This state will eventually be accomodated with  additional settings (ex: change the Routing strategy..) before being sent to the 
simulator.

*/

public:
    const double getDistanceThreshold() const { return distanceThreshold; }
    const std::string& getTopologyLines() const { return topologyLines; }



    void setDistanceThreshold(double newThreshold) {
        distanceThreshold = newThreshold;
    }

    void setTopologyLines(std::string newLines) {
    topologyLines = std::move(newLines);
    }

    void resetState()
    {
        distanceThreshold = 0.0; // Default distance threshold
        topologyLines.clear();
    };

    void setScenarioType(ScenarioType type) {
        scenarioType = type;
    }
    ScenarioType getScenarioType() const {
        return scenarioType;
    }

    
    
private:
        double distanceThreshold; // Default distance threshold
    std::string topologyLines;
    ScenarioType scenarioType = ScenarioType::SimonV1;
};

