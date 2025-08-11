#pragma once

#include <SFML/Network.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <iostream>

class TcpServer {

    /*
    Manages the tcp server for bidirectional communication
    */
public:
    using PacketHandler = std::function<void(sf::Packet&)>;

    explicit TcpServer();
    ~TcpServer();

    void start(unsigned short port);
    void stop();
    std::string getClientStatus();
    void setPacketHandler(PacketHandler handler);
 // New: Send to all clients
    void transmitPacket( sf::Packet& packet);
private:
    void processPacket(sf::Packet& packet);
    std::atomic<bool> running{false};
    std::thread serverThread;
    PacketHandler packetHandler;
    std::unique_ptr<sf::TcpSocket> client;
    std::mutex clientMutex;
};