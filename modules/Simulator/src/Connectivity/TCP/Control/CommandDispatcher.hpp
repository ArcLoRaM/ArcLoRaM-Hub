#pragma once

#include <SFML/Network/Packet.hpp>
#include <string>
#include <vector>
#include <optional>
#include <mutex>
#include "../../Logger/Logger.hpp"

struct LaunchConfig {
    double distanceThreshold;
    std::string communicationMode;
    std::string topology;
    //optionnal for now
    std::vector<std::string> nodeLines;
};

class CommandDispatcher {
public:
    explicit  CommandDispatcher(Logger& logger);

    void onCommand(sf::Packet& packet);

    std::optional<LaunchConfig> getPendingLaunchConfig(); // For main() to use
    void clear();

private:
    std::optional<LaunchConfig> pendingConfig;
    std::mutex configMutex;

    Logger& logger; // Reference to the logger for logging commands


    void handleLaunchConfigCommand( sf::Packet& packet);
};
