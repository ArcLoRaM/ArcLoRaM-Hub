#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <map>
#include <fstream>
#include <variant>
#include <iostream>
#include <atomic>
#include "LogSeverity.hpp"

class LogProcessor {
public:
    struct NodeInfo {
        int id;
        int cls; // 1 = E-Node, 2 = Relay, 3 = Gateway
    };

    LogProcessor() = default;
    ~LogProcessor() = default;

    // Lifecycle management
    void start();
    void stop();

    // Logging operations
    void logEvent(int nodeId, const std::string& message);
    void logMessage(LogSeverity severity, const std::string& message);
    void setCurrentTick(uint64_t tick);
    void resetTick();

    // Configuration
    void enableFileOutput(const std::string& filepath);
    void setNodes(const std::vector<NodeInfo>& nodeInfoList);
    void enableColorOutput(bool enabled);

private:
    inline static std::atomic<uint64_t> globalLogSeq{0};

    struct LogEntry {
        uint64_t tick;
        int nodeId;
        std::string message;
    };

    struct SeverityLog {
        LogSeverity severity;
        std::string message;
    };

    using LogVariant = std::variant<LogEntry, SeverityLog>;

    // Processing
    void processLogs();
    void flushTick(uint64_t tick, const std::map<int, std::vector<std::string>>& nodeLogs);
    std::string joinMessages(const std::vector<std::string>& messages, const std::string& delimiter);
    std::string formatNodeLabel(int nodeId);
    std::string formatSeverityLog(LogSeverity severity, const std::string& message);

    // State
    std::queue<LogVariant> logQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    bool stopFlag = false;
    std::thread loggerThread;

    std::ofstream fileStream;
    size_t columnWidth = 25;
    bool useColor = false;

    std::vector<NodeInfo> nodes;
    uint64_t currentTick = 0;
    std::map<int, std::vector<std::string>> pendingNodeLogs;
};