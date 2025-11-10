#pragma once

#include <SFML/Network/Packet.hpp>
#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include "../../Logger/Logger.hpp"  
#include <functional>


/*
Brief: CommandDispatcher is responsible for handling incoming commands and managing the launch configuration.  
    Need to set the callbacks for simulation control commands.
*/


struct LaunchConfig {
    double distanceThreshold;
    std::string communicationMode;
    std::string topologyLines;
    //idea for later:
    //SF, BW, PathLoss Model...
};

class CommandDispatcher {
public:
    explicit  CommandDispatcher(Logger& logger);
    using StopCallback = std::function<void()>;
    using PingCallback = std::function<void()>;
    using RestartCallback = std::function<void()>;
    using PauseCallback = std::function<void()>;
    using ResumeCallback = std::function<void()>;

    void onCommand(sf::Packet& packet);

    std::optional<LaunchConfig> getPendingLaunchConfig(); 
    void setStopCallback(StopCallback callback);

    void clearConfig();
    void setPingCallback(PingCallback callback);
    void setRestartCallback(RestartCallback callback);
    void setPauseCallback(PauseCallback callback);
    void setResumeCallback(ResumeCallback callback);

private:

    std::optional<LaunchConfig> pendingConfig;
    std::mutex configMutex;
    Logger& logger; // Reference to the logger for logging commands
    void handleLaunchConfigCommand( sf::Packet& packet);
    StopCallback stopCallback;
    PingCallback pingCallback;
    RestartCallback restartCallback;
    PauseCallback pauseCallback;
    ResumeCallback resumeCallback;

};
