#pragma once

#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <iostream>

class TcpServer {
public:
    using PacketHandler = std::function<void(sf::Packet&)>;

    explicit TcpServer();
    ~TcpServer();

    void start(unsigned short port);
    void stop();

    void setPacketHandler(PacketHandler handler);

private:
    void runLoop();
    void processPacket(sf::Packet& packet);

    std::atomic<bool> running{false};
    std::thread serverThread;
    PacketHandler packetHandler;
};
