
#include "Logger.hpp"
#include "../../Node/Node.hpp"

void Logger::start() {
    stopFlag = false;
    tcpStopFlag = false;
    loggerThread = std::thread(&Logger::processLogs, this);

    if (common::visualiserConnected) {
        tcpSenderThread = std::thread(&Logger::processTcpPackets, this);
    }
}


void Logger::stop() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        stopFlag = true;
        tcpStopFlag = true;
    }
    cv.notify_all();

    if (loggerThread.joinable()) loggerThread.join();
    if (common::visualiserConnected && tcpSenderThread.joinable()) {
        tcpSenderThread.join();
    }
    if (fileStream.is_open()) fileStream.close();
}

void Logger::sendTcpPacket(sf::Packet packet) {
    if (!common::visualiserConnected) return;

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        if (packet) {
            tcpQueue.push(packet);
        }
    }
    cv.notify_one();
}

void Logger::logEvent(int nodeId, const std::string& message) {
    std::lock_guard<std::mutex> lock(queueMutex);
    uint64_t seq = globalLogSeq++;
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << seq << ": " << message;
    pendingNodeLogs[nodeId].emplace_back(oss.str());
}

void Logger::setCurrentTick(uint64_t tick) {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (tick > currentTick) {
        if (!pendingNodeLogs.empty()) {
            flushTick(currentTick, pendingNodeLogs);
            pendingNodeLogs.clear();
        }
        currentTick = tick;
    }
}


void Logger::logSystem(const std::string& message) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        logQueue.push(SystemLog{message});
    }
    cv.notify_one();
}


void Logger::exportCombinedSchedule(
    const std::vector<std::shared_ptr<Node>>& nodes,
    const std::multimap<int64_t, std::shared_ptr<Node>>& communicationSteps,
    const std::string& outputFile)
{
    // Map: timestamp -> nodeId -> info
    std::map<int64_t, std::map<int, std::string>> scheduleTable;

    // 1. Collect state transitions
    for (const auto& node : nodes) {
        int nodeId = node->getId();
        for (const auto& [time, state] : node->getActivationSchedule()) {
            scheduleTable[time][nodeId] = Node::stateToString(state);
        }
    }

    // 2. Collect communication steps
    for (const auto& [time, nodePtr] : communicationSteps) {
        if (!nodePtr) continue;
        int nodeId = nodePtr->getId();
        auto& entry = scheduleTable[time][nodeId];
        if (!entry.empty()) {
            entry += "+Comm";
        } else {
            entry = "Comm";
        }
    }

    // 3. Write to CSV
    std::ofstream out(outputFile);
    if (!out.is_open()) {
        std::cerr << "Failed to open " << outputFile << "\n";
        return;
    }

    // Header
    out << "Timestamp";
    for (const auto& node : nodes) {
        out << ",Node " << node->getId();
    }
    out << "\n";

    // Rows
    for (const auto& [time, nodeMap] : scheduleTable) {
        out << time;
        for (const auto& node : nodes) {
            int id = node->getId();
            auto it = nodeMap.find(id);
            if (it != nodeMap.end()) {
                out << "," << it->second;
            } else {
                out << ",";
            }
        }
        out << "\n";
    }

    out.close();
}


void Logger::setTcpClient(Client* clientPtr) {
    client = clientPtr;
}

void Logger::enableFileOutput(const std::string& filepath) {
    fileStream.open(filepath);
}

void Logger::setNodes(const std::vector<NodeInfo>& nodeInfoList) {
    nodes = nodeInfoList;
    std::sort(nodes.begin(), nodes.end(), [](const NodeInfo& a, const NodeInfo& b) {
        return a.id < b.id;
    });
}

void Logger::enableColorOutput(bool enabled)
{
    useColor = enabled;
    
}

std::string Logger::formatNodeLabel(int nodeId) {
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


void Logger::processLogs() {
    std::map<uint64_t, std::map<int, std::vector<std::string>>> tickLogs;

    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);
        cv.wait(lock, [this] { return !logQueue.empty() || stopFlag; });

        while (!logQueue.empty()) {
            auto item = logQueue.front();
            logQueue.pop();

            std::visit([&](auto &&log)
                       {
                using T = std::decay_t<decltype(log)>;
                if constexpr (std::is_same_v<T, LogEntry>) {
                    tickLogs[log.tick][log.nodeId].push_back(log.message);
                } else if constexpr (std::is_same_v<T, SystemLog>) {
                    std::string sysLine = "[SYSTEM] " + log.message;

                    if (useColor)
                    {
                        std::cout << "\033[31m" << sysLine << "\033[0m\n";
                    }
                    else
                    {
                        std::cout << sysLine << "\n";
                    }

                    if (fileStream.is_open())
                    {
                        fileStream << sysLine << "\n"; // never add color to file
                    }
                } }, item);
        }
        // when we stop the logger, we flush all pending logs
        if (stopFlag && logQueue.empty())
        {
            // Flush tick-based logs
            for (const auto &[tick, logs] : tickLogs)
            {
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

void Logger::flushTick(uint64_t tick, const std::map<int, std::vector<std::string>>& nodeLogs) {
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

std::string Logger::joinMessages(const std::vector<std::string>& messages, const std::string& delimiter) {
    std::ostringstream oss;
    for (size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) oss << delimiter;
        oss << messages[i];
    }
    return oss.str();
}

void Logger::processTcpPackets() {
    while (true) {
        sf::Packet packet;

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            // Exit condition
            if (tcpStopFlag && tcpQueue.empty()) break;

            // Wait for packet or stop signal
            cv.wait(lock, [this] { return !tcpQueue.empty() || tcpStopFlag; });

            if (!tcpQueue.empty()) {
                packet = tcpQueue.front(); // Copy the front packet for processing outside the lock
            } else {
                continue; // Spurious wakeup or stop flag without packets
            }
        }

        // Attempt to send outside the lock
        if (client && client->transmit(packet)) {
            std::lock_guard<std::mutex> lock(queueMutex);
            tcpQueue.pop(); // Remove only on successful send
        } else {
            // Retry after a short wait if the visualiser is still connected
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (!common::visualiserConnected) {
                std::lock_guard<std::mutex> lock(queueMutex);
                tcpQueue.pop(); // Drop if visualiser is no longer active
            }
        }
    }
}

