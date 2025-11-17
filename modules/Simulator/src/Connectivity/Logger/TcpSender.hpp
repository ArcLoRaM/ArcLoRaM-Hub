#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <SFML/Network.hpp>
#include "../TCP/Client/Client.hpp"

class TcpSender {
public:
    TcpSender() = default;
    ~TcpSender() = default;

    // Lifecycle management
    void start();
    void stop();

    // TCP operations
    void sendPacket(sf::Packet packet);
    void setClient(Client* clientPtr);

private:
    void processTcpPackets();

    // TCP queue and thread
    std::queue<sf::Packet> tcpQueue;
    std::thread tcpSenderThread;
    std::mutex tcpMutex;
    std::condition_variable tcpCv;
    bool tcpStopFlag = false;

    Client* client = nullptr;
};