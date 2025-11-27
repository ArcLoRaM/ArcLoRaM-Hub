#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <memory>
#include "../../Connectivity/Logger/Logger.hpp"
#include <string>
#include "../../Connectivity/TCP/Packets/Packets.hpp"
#include "../Node.hpp"
#include "../../Setup/Common.hpp"

#include <stop_token>     // C++20

class MetricsAggregator; // Forward declaration
class MetricsExporter;   // Forward declaration

using CallbackType = std::function<void()>;



class Clock {
public:

    using ms = std::chrono::milliseconds;


 Clock(Logger& logger, ms tickPeriod = ms{1});


    ~Clock();

    // Lifecycle
    void start() ;
    void stop();

    // Control
    void pause();
    void resume();
    bool isPaused()  const noexcept { return paused.load(std::memory_order_relaxed); }
    bool isRunning() const noexcept { return running.load(std::memory_order_relaxed); }
    
    
    // Time (logical)
    int64_t currentTimeInMilliseconds();


    void scheduleStateTransition(int64_t activationTimeMs, CallbackType callback) ;
    void scheduleCommunicationStep(int64_t timeMs, std::shared_ptr<Node> node);
    void scheduleTransmissionStart(int64_t timeMs, CallbackType callback);
    void scheduleTransmissionEnd(int64_t timeMs, CallbackType callback);

    // Metrics integration
    void setMetricsAggregator(MetricsAggregator* aggregator) { metricsAggregator = aggregator; }
    std::multimap<int64_t, std::shared_ptr<Node>> getCommunicationStepsSnapshot() const; 



private:

    //thread loop
    void run(std::stop_token stoken);
    std::jthread clockThread;  //auto-join stop

    //helpers
    void executeCallbacksInRange(std::multimap<int64_t, CallbackType>& map, int64_t start, int64_t end);
    void executeCommunicationInRange(std::multimap<int64_t,std::shared_ptr<Node>>& map, int64_t start, int64_t end);

    //State
    std::atomic<bool> running{false};
    std::atomic<bool> paused{false};
    Logger& logger;
    mutable std::mutex clockMutex;
    std::condition_variable_any cv;

    // Metrics
    MetricsAggregator* metricsAggregator = nullptr;
    std::unique_ptr<MetricsExporter> metricsExporter;

    //Schedules
    //use distinct multimap for every kind of events (battery depletion etc..)
    std::multimap<int64_t, CallbackType> stateTransitions; //stores the calls of onTimeChange() for each node at the activation times
                                                             //onTimeChange() will call the appropriate stateTransitionFunction
    std::multimap<int64_t, std::shared_ptr<Node>> communicationSteps;//consists of the HandleCommunication() for each node, is provisionned at the same schedule than the stateTransitions multimap
    
    std::multimap<int64_t, CallbackType> transmissionStartCallbacks;
    std::multimap<int64_t, CallbackType> transmissionEndCallbacks;      
    
    //we can add more multimaps for other events, like  special events (sudden node failure etc..)
    
    ms tickPeriod{1};
    uint64_t tNowMs{0};                   // logical time
    uint64_t tLastMs{0};
    
    uint64_t tickCount{0};




   
};

