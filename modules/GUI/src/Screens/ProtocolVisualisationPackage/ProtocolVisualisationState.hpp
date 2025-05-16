#pragma once

#include <string>
#include <vector>
#include <mutex>


struct ProtocolVisualisationState {
    std::string communicationMode = "None";
    //Todo: do we need to protec the states with mutexes? all of them?
    int tickNumber = 0;
    float distanceThreshold = 0.f;
    int retransmissions = 0;
    int energyExp = 0;
    //the logs will overflow because you erased: 

        // while (state.logMessages.size() > 10)
        //     state.logMessages.erase(state.logMessages.begin());
    std::vector<std::string> logMessages;
    mutable std::mutex logMutex; // Mutex for thread-safe access to log messages
};