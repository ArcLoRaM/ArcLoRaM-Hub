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

    // --- Singleton access ---
    static TcpServer& instance() {
        static TcpServer inst;
        return inst;
    }

    // No copying/moving
    TcpServer(const TcpServer&)            = delete;
    TcpServer& operator=(const TcpServer&) = delete;
    TcpServer(TcpServer&&)                 = delete;
    TcpServer& operator=(TcpServer&&)      = delete;

    //API
    void start(unsigned short port);
    void stop();
    void setPacketHandler(PacketHandler handler);
    void transmitPacket( sf::Packet& packet);
    void updateLastPong();

private:
    // Only the singleton can construct/destroy
    TcpServer() = default;
    ~TcpServer() { stop(); }

    void processPacket(sf::Packet& packet);
        void tickHeartbeat();

    std::atomic<bool> running{false};
    std::thread serverThread;
    PacketHandler packetHandler;
    
    std::unique_ptr<sf::TcpSocket> client;
    std::mutex clientMutex;


    //heartbeat to check if client is still connected  
    std::chrono::steady_clock::time_point lastPong{std::chrono::steady_clock::now()};
    std::chrono::steady_clock::time_point lastPing{std::chrono::steady_clock::now()};
    const std::chrono::seconds pingInterval{5};
    const std::chrono::seconds pongTimeout{15};
};