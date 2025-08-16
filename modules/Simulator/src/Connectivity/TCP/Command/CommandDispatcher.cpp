#include "CommandDispatcher.hpp"
#include <filesystem>
#include <iostream>
#include "../Packets/Packets.hpp"

CommandDispatcher::CommandDispatcher(Logger& logger)
    : logger(logger)
{}

void CommandDispatcher::onCommand(sf::Packet& packet) {
    int type=-1;
    packet >> type;

    switch (type) {
        case 100: {
            logger.logSystem("Configuration received.");
            handleLaunchConfigCommand(packet);                
            break;
        }

        case 101:
            logger.logSystem("StopSimulation command received.");
            if (stopCallback) {
                stopCallback();
            }
            break;

        case 102:
            logger.logSystem("ResumeSimulation command received.");
            // if (resumeCallback) resumeCallback();
            break;

        case 103:
            // logger.logSystem("Ping command received.");
            if (pingCallback) pingCallback();
            break;

        case 105:
            logger.logSystem("RestartSimulation command received.");
            if (restartCallback) restartCallback();
            break;

        default:
            logger.logSystem("Unknown command type received: " + std::to_string(type));
            break;
    }
}

void CommandDispatcher::handleLaunchConfigCommand( sf::Packet& packet)
{
            launchConfigCommandPacket cmd;
            packet >> cmd;

two options:
-either we decouple the topology from the general simulation parameter. THe packet will be composed of different variables plus "topologyLines"
-or we pass everything as a string and we rely on the existing parsing logic already implemented


advantages:
1)Clear separation of the network and what's around + seems easier to do with the current implementation
2)if the number of parameters grows, it's easier to manage them with a string??


            {
                std::lock_guard lock(configMutex);
                pendingConfig = LaunchConfig{
                    cmd.distanceThreshold,
                    cmd.communicationMode,
                    //cmd.topologyLines
                };
            }

            logger.logSystem("Received launch config. Mode: " + cmd.communicationMode
                             + " | Threshold: " + std::to_string(cmd.distanceThreshold));
                            //  + " | Nodes: " + std::to_string(cmd.nodeLines.size()))

}

std::optional<LaunchConfig> CommandDispatcher::getPendingLaunchConfig() {
    std::lock_guard lock(configMutex);
    return pendingConfig;
}

void CommandDispatcher::clear() {
    std::lock_guard lock(configMutex);
    pendingConfig.reset();
}

void CommandDispatcher::setStopCallback(StopCallback callback) {
    stopCallback = std::move(callback);
}

void CommandDispatcher::setPingCallback(PingCallback callback) {
    pingCallback = std::move(callback);
}

void CommandDispatcher::setRestartCallback(RestartCallback callback) {
    restartCallback = std::move(callback);
}