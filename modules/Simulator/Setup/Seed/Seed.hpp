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



struct C2_Node_Mesh_Parameter{
    std::pair<int,int> coordinates;
    unsigned int hopCount;
    uint16_t nextNodeIdInPath;
};

class Seed {

public:
    Seed(std::string communicationWindow, std::string topology,Logger& logger) : 
    communicationWindow(communicationWindow), logger(logger), 
    topology(topology) {

        initializeNodes();
        
    }

    std::vector<std::shared_ptr<Node>> transferOwnership();
  

private:
    Logger& logger;
    std::string communicationWindow;
    std::string topology;
    std::vector<std::shared_ptr<Node>> listNode;

    int64_t  baseTime;

    void initializeNodes();

        //TODO: there is bug: the first node must have an ID=0, (we should be able to attribute any ID to the nodes)-> I didn't check this for a long time, maybe it's not the case anymore?
    #if COMMUNICATION_PERIOD == RRC_DOWNLINK || COMMUNICATION_PERIOD == RRC_BEACON

    void initialize_RRC_Downlink_Line();
    void initialize_RRC_Beacon_Line();
    void initialize_RRC_Beacon_Mesh();
    void initialize_RRC_Beacon_Mesh_Self_Healing();
    #elif COMMUNICATION_PERIOD == RRC_UPLINK

    void initialize_RRC_Uplink_Mesh();
    void initialize_RRC_Uplink_Mesh_FromFile();

    void initialize_RRC_Uplink_Line();
    void initialize_RRC_Uplink_Line_FromFile();
    #endif
};
