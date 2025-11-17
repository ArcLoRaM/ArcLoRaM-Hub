#include "LogProcessor.hpp"
#include <iomanip>
#include <sstream>
#include <algorithm>

void LogProcessor::start() {
    stopFlag = false;
    loggerThread = std::thread(&LogProcessor::processLogs, this);
}

void LogProcessor::stop() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopFlag = true;
    }
    cv.notify_all();

    if (loggerThread.joinable()) {
        loggerThread.join();
    }

    if (fileStream.is_open()) {
        fileStream.close();
    }
}

void LogProcessor::logEvent(int nodeId, const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    uint64_t seq = globalLogSeq++;
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << seq << ": " << message;
    pendingNodeLogs[nodeId].emplace_back(oss.str());
}

void LogProcessor::logMessage(LogSeverity severity, const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(SeverityLog{severity, message});
    }
    cv.notify_one();
}

void LogProcessor::setCurrentTick(uint64_t tick) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (tick > currentTick) {
        if (!pendingNodeLogs.empty()) {
            flushTick(currentTick, pendingNodeLogs);
            pendingNodeLogs.clear();
        }
        currentTick = tick;
    }
}

void LogProcessor::resetTick() {
    currentTick = 0;
}

void LogProcessor::enableFileOutput(const std::string& filepath) {
    fileStream.open(filepath);
}

void LogProcessor::setNodes(const std::vector<NodeInfo>& nodeInfoList) {
    nodes = nodeInfoList;
    std::sort(nodes.begin(), nodes.end(), [](const NodeInfo& a, const NodeInfo& b) {
        return a.id < b.id;
    });
}

void LogProcessor::enableColorOutput(bool enabled) {
    useColor = enabled;
}

std::string LogProcessor::formatNodeLabel(int nodeId) {
    static const std::map<int, std::string> classNames = {
        {1, "EN"}, {2, "RL"}, {3, "GW"}
    };
    static const std::map<int, std::string> classColors = {
        {1, "\033[32m"}, {2, "\033[36m"}, {3, "\033[35m"}
    };

    auto it = std::find_if(nodes.begin(), nodes.end(), [&](const NodeInfo& n) {
        return n.id == nodeId;
    });
    int cls = (it != nodes.end()) ? it->cls : 2; // default to Relay

    std::ostringstream label;
    if (useColor) label << classColors.at(cls);
    label << classNames.at(cls) << " " << nodeId;
    if (useColor) label << "\033[0m";
    return label.str();
}

std::string LogProcessor::formatSeverityLog(LogSeverity severity, const std::string& message) {
    std::ostringstream oss;

    if (useColor) {
        oss << LogSeverityHelper::getColorCode(severity);
    }

    oss << "[" << LogSeverityHelper::toString(severity) << "] " << message;

    if (useColor) {
        oss << LogSeverityHelper::getResetCode();
    }

    return oss.str();
}

void LogProcessor::processLogs() {
    std::map<uint64_t, std::map<int, std::vector<std::string>>> tickLogs;

    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !logQueue.empty() || stopFlag; });

        while (!logQueue.empty()) {
            auto item = logQueue.front();
            logQueue.pop();

            std::visit([&](auto&& log) {
                using T = std::decay_t<decltype(log)>;
                if constexpr (std::is_same_v<T, LogEntry>) {
                    tickLogs[log.tick][log.nodeId].push_back(log.message);
                } else if constexpr (std::is_same_v<T, SeverityLog>) {
                    std::string logLine = formatSeverityLog(log.severity, log.message);
                    std::cout << logLine << "\n";

                    if (fileStream.is_open()) {
                        // Strip color codes for file output
                        std::string fileLog = "[" + LogSeverityHelper::toString(log.severity) + "] " + log.message;
                        fileStream << fileLog << "\n";
                    }
                }
            }, item);
        }

        // When we stop the logger, flush all pending logs
        if (stopFlag && logQueue.empty()) {
            // Flush tick-based logs
            for (const auto& [tick, logs] : tickLogs) {
                flushTick(tick, logs);
            }
            // Flush node logs
            if (!pendingNodeLogs.empty()) {
                flushTick(currentTick, pendingNodeLogs);
                pendingNodeLogs.clear();
            }
            break;
        }

        while (!tickLogs.empty()) {
            auto it = tickLogs.begin();
            flushTick(it->first, it->second);
            tickLogs.erase(it);
        }
    }
}

void LogProcessor::flushTick(uint64_t tick, const std::map<int, std::vector<std::string>>& nodeLogs) {
    std::ostringstream oss;
    if (useColor) {
        oss << "\033[90m"; // Neutral gray
    }
    oss << "[T: " << std::setw(6) << std::setfill('0') << tick << "]";
    if (useColor) {
        oss << "\033[0m"; // Reset after time block
    }
    oss << std::setfill(' '); // Restore fill for node labels

    for (const auto& node : nodes) {
        oss << " | " << std::setw(10) << std::left << formatNodeLabel(node.id) << ": ";
        auto it = nodeLogs.find(node.id);
        if (it != nodeLogs.end()) {
            oss << std::setw(columnWidth) << std::left << joinMessages(it->second, ", ");
        } else {
            oss << std::setw(columnWidth) << " ";
        }
    }

    std::string line = oss.str();
    std::cout << line << "\n";
    if (fileStream.is_open()) {
        fileStream << line << "\n";
    }
}

std::string LogProcessor::joinMessages(const std::vector<std::string>& messages, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << messages[i];
    }
    return oss.str();
}