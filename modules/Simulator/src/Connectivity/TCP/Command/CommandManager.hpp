#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <optional>
#include "Connectivity/TCP/Command/CommandDispatcher.hpp"
#include "Connectivity/TCP/Client/Client.hpp"
#include "Connectivity/Logger/Logger.hpp"
#include "Node/Clock/Clock.hpp"
#include "PhyLayer/PhyLayer.hpp"
#include "Metrics/MetricsAggregator.hpp"


/*

Holds the dispatcher and the listener, initialize them with the appropriate callbacks

*/


class CommandManager {
public:
    explicit CommandManager(Logger& logger);
    ~CommandManager();

    void start();          // Start listening and wait for config
    void stop();           // Stop listener and simulation
    bool isRunning() const;
    bool initialized;
private:
    void waitForLaunchConfig();
    std::atomic_flag waitingForConfig = ATOMIC_FLAG_INIT;

    void launchSimulation(const LaunchConfig& config);
    void stopSimulation();


    Logger& logger;
    Client tcpClient;
    CommandDispatcher dispatcher;
    std::atomic<bool> running;

    // Simulation components (destruction order matters!)
    std::unique_ptr<MetricsAggregator> metricsAggregator;
    std::unique_ptr<Clock> clock;
    std::unique_ptr<PhyLayer> phyLayer;

};
