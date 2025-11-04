
#pragma once

#include <unordered_map>
#include <vector>
#include <optional>
#include <string>
#include <mutex>
#include <memory>
#include <SFML/System/Vector2.hpp>
#include "../../Objects/Device/Device.hpp"

enum class TDMAMode {
    RRC_Uplink,
    RRC_Downlink,
    RRC_Beacon,
    ENC_Downlink,
    ENC_Uplink,
    ENC_Beacon,
    No_Mode
};



class TopologyFileState {

/*
Holds the state of the parsed topology, from the configuration file.
This state will eventually be accomodated with  additional settings (ex: change the Routing strategy..) before being sent to the 
simulator.

*/

public:
    const TDMAMode getTDMAMode()const {return currentMode; } 
    const double getDistanceThreshold() const { return distanceThreshold; }
    const std::string& getTopologyLines() const { return topologyLines; }

    void setTDMAMode(TDMAMode newMode) {
        currentMode = newMode;
    }

    void setDistanceThreshold(double newThreshold) {
        distanceThreshold = newThreshold;
    }

    void setTopologyLines(std::string newLines) {
    topologyLines = std::move(newLines);
    }

    void resetState()
    {
        currentMode = TDMAMode::No_Mode; // Default mode
        distanceThreshold = 0.0; // Default distance threshold
        topologyLines.clear();
    };


    
    
private:
    

    TDMAMode currentMode;
    double distanceThreshold; // Default distance threshold
    std::string topologyLines;
};

