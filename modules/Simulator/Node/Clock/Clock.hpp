#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include "../../Connectivity/Logger/Logger.hpp"
#include <string>
#include "../../Connectivity/TCP/packets.hpp"
#include "../Node.hpp"


using CallbackType = std::function<void()>;



class Clock {


private:
    std::atomic<bool> running;


    //use distinct multimap for every kind of events (battery depletion etc..)
    std::multimap<int64_t, CallbackType> stateTransitions; //stores the calls of onTimeChange() for each node at the activation times
                                                             //onTimeChange() will call the appropriate stateTransitionFunction
    std::multimap<int64_t, std::shared_ptr<Node>> communicationSteps;//consists of the HandleCommunication() for each node, is provisionned at the same schedule than the stateTransitions multimap
    
    //not sure if we will use multimaps for dispatching packets, interference etc..
    std::multimap<int64_t, CallbackType> transmissionStartCallbacks;
    std::multimap<int64_t, CallbackType> transmissionEndCallbacks;      
    
    //we can add more multimaps for other events, like  special events (sudden node failure etc..)
    
    int64_t logicalTimeMs = 0;  // Start at 0
    const int64_t tickDurationMs = common::tickIntervalForClock_ms;                                                            
    int64_t lastProcessedTime = 0;


    std::thread clockThread;
    Logger& logger;
    
    void tick();
    unsigned int compteurTick=0;

    void executeCallbacksInRange(std::multimap<int64_t, CallbackType>& map, int64_t start, int64_t end);
    void executeCommunicationInRange(std::multimap<int64_t,std::shared_ptr<Node>>& map, int64_t start, int64_t end);


public:
    Clock( Logger& logger) 
        : running(false),logger(logger) {
        }

    void start() ;

    void stop();
    // Current time in milliseconds
    int64_t currentTimeInMilliseconds() ;
    void scheduleStateTransition(int64_t activationTime, CallbackType callback) ;
    void scheduleCommunicationStep(int64_t time, std::shared_ptr<Node> node);
    void scheduleTransmissionStart(int64_t time, CallbackType callback);
    void scheduleTransmissionEnd(int64_t time, CallbackType callback);

};

