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
            if (resumeCallback) resumeCallback();
            break;

        case 103:
            // logger.logSystem("Ping command received.");
            if (pingCallback) pingCallback();
            break;

        case 105:
            logger.logSystem("RestartSimulation command received.");
            if (restartCallback) restartCallback();
            break;
        case 106:
            logger.logSystem("PauseSimulation command received.");
            if (pauseCallback) pauseCallback();
            break;

        default:
            logger.logSystem("Unknown command type received: " + std::to_string(type));
            break;
    }
}

void CommandDispatcher::handleLaunchConfigCommand( sf::Packet& packet)
{
            launchConfigCommandPacket cmd(0.0,"",0);
            packet >> cmd;

            {
                std::lock_guard lock(configMutex);
                pendingConfig = LaunchConfig{
                    cmd.distanceThreshold,
                    cmd.topologyString,
                    static_cast<ScenarioType>(cmd.scenarioType)
                };
            }

            logger.logSystem("Scenario: " + std::to_string(cmd.scenarioType)
                             + " | Threshold: " + std::to_string(cmd.distanceThreshold)
                             );

}
    
std::optional<LaunchConfig> CommandDispatcher::getPendingLaunchConfig() {
    std::lock_guard lock(configMutex);
    return pendingConfig;
}

void CommandDispatcher::clearConfig() {
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

void CommandDispatcher::setPauseCallback(PauseCallback callback) {
    pauseCallback = std::move(callback);
}

void CommandDispatcher::setResumeCallback(ResumeCallback callback) {
    resumeCallback = std::move(callback);
}