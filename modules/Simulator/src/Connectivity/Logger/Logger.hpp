

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
#include <SFML/Network.hpp> // Assuming this is available
#include "../TCP/Client/Client.hpp" // Assuming this is available

//TODO: this class is too big, make at least two: one class-one duty

class Node; // Forward declaration of Node class

class Logger {
public:
    struct NodeInfo {
    int id;
    //TODO: use the enum class in NodeEnums.hpp?
    int cls; // 1 = E-Nod, 2 = Relay, 3 = GtWay
};

    void start();
    void stop();

    //TODO: Make a logError (right now uses logSystem)
    void logEvent(int nodeId, const std::string& message);
    void logSystem(const std::string& message);

    void sendTcpPacket(sf::Packet packet);
    void setTcpClient(Client* clientPtr);
    void enableFileOutput(const std::string& filepath);
    void setCurrentTick(uint64_t tick);

    void setNodes(const std::vector<NodeInfo>& nodeInfoList);
    void enableColorOutput(bool enabled);
    void exportCombinedSchedule(
        const std::vector<std::shared_ptr<Node>>& nodes,
        const std::multimap<int64_t, std::shared_ptr<Node>>& communicationSteps,
        const std::string& outputFile = "combined_schedule.csv"
    );
    void resetTick();

private:
inline static std::atomic<uint64_t> globalLogSeq{0};
    struct LogEntry {
        uint64_t tick;
        int nodeId;
        std::string message;
    };

    struct SystemLog {
        std::string message;
    };

std::string formatNodeLabel(int nodeId);
    bool useColor = false;            // toggle for color output
std::vector<NodeInfo> nodes;

    using LogVariant = std::variant<LogEntry, SystemLog>;

    void processLogs();
    void flushTick(uint64_t tick, const std::map<int, std::vector<std::string>>& nodeLogs);
    std::string joinMessages(const std::vector<std::string>& messages, const std::string& delimiter);

    std::queue<LogVariant> logQueue;
    std::queue<sf::Packet> tcpQueue;

    std::mutex queueMutex;
    std::condition_variable cv;
    bool stopFlag = false;
    std::thread loggerThread;

    std::ofstream fileStream;
    size_t columnWidth = 25;

    Client* client = nullptr; // external TCP client

    uint64_t currentTick = 0;
    std::map<int, std::vector<std::string>> pendingNodeLogs;

    //TCP
    std::thread tcpSenderThread;
    void processTcpPackets();

    bool tcpStopFlag = false;
};
