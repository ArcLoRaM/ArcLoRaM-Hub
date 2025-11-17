
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "LogProcessor.hpp"
#include "TcpSender.hpp"
#include "LogSeverity.hpp"
#include <SFML/Network.hpp>
#include "../TCP/Client/Client.hpp"

class Node; // Forward declaration

/**
 * Logger - Unified entry point for logging and TCP transmission
 *
 * This class acts as a facade that coordinates:
 * - LogProcessor: Handles console/file logging with severity levels
 * - TcpSender: Handles TCP packet transmission
 */
class Logger {
public:
    using NodeInfo = LogProcessor::NodeInfo;

    Logger();
    ~Logger() = default;

    // Lifecycle management
    void start();
    void stop();

    // ===== LOGGING API =====
    // Node event logging (tick-based)
    void logEvent(int nodeId, const std::string& message);

    // Severity-based logging
    void logDebug(const std::string& message);
    void logInfo(const std::string& message);
    void logWarning(const std::string& message);
    void logError(const std::string& message);
    void logCritical(const std::string& message);
    void logSystem(const std::string& message);

    // Generic severity logging
    void log(LogSeverity severity, const std::string& message);

    // Tick management
    void setCurrentTick(uint64_t tick);
    void resetTick();

    // ===== TCP API =====
    void sendTcpPacket(sf::Packet packet);
    void setTcpClient(Client* clientPtr);

    // ===== CONFIGURATION =====
    void enableFileOutput(const std::string& filepath);
    void setNodes(const std::vector<NodeInfo>& nodeInfoList);
    void enableColorOutput(bool enabled);

    // Legacy/deprecated - to be removed or reimplemented
    void exportCombinedSchedule(
        const std::vector<std::shared_ptr<Node>>& nodes,
        const std::multimap<int64_t, std::shared_ptr<Node>>& communicationSteps,
        const std::string& outputFile = "combined_schedule.csv"
    );

private:
    std::unique_ptr<LogProcessor> logProcessor;
    std::unique_ptr<TcpSender> tcpSender;
};
