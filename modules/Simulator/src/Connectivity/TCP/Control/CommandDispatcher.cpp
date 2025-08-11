#include "CommandDispatcher.hpp"
#include <filesystem>
#include <iostream>
#include "../packets.hpp"

CommandDispatcher::CommandDispatcher(Logger& logger)
    : logger(logger)
{}

void CommandDispatcher::onCommand(sf::Packet& packet) {
    int type=-1;
    packet >> type;

    switch (type) {
        case 101: {
            logger.logSystem("Configuration received.");
            handleLaunchConfigCommand(packet);                
            break;
        }
        case 102:
            logger.logSystem("StopSimulation command received.");
            if (stopCallback) {
                stopCallback();
            }
            break;
        case 103:
            logger.logSystem("Ping command received.");
            if (pingCallback) pingCallback();
            break;

        case 104:
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

            {
                std::lock_guard lock(configMutex);
                pendingConfig = LaunchConfig{
                    cmd.distanceThreshold,
                    cmd.communicationMode,
                    cmd.topology
                    //cmd.nodeLines
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