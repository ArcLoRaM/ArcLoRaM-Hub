#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <atomic>

class VisualiserManager;

class NetworkManager {
public:
    //Suppress RenderWindow when you have the new inpuut system.
    explicit NetworkManager(VisualiserManager& manager,sf::RenderWindow& window);
    ~NetworkManager();

    void stop();

    // No copying allowed
    NetworkManager(const NetworkManager&) = delete;
    NetworkManager& operator=(const NetworkManager&) = delete;

private:
    VisualiserManager& manager;
    //suppress RenderWindow when you have the new inpuut system.
sf::RenderWindow& window;

    std::atomic<bool> running;
    std::thread networkThread;


    void runLoop();
    void processPacket(sf::Packet& packet);

    void handleSystemPacket(sf::Packet& packet);
    void handleTickPacket(sf::Packet& packet);
    void handleStateNodePacket(sf::Packet& packet);
    void handlePositionPacket(sf::RenderWindow & window,sf::Packet& packet);
    void handleTransmitMessagePacket(sf::Packet& packet);
    void handleReceiveMessagePacket(sf::Packet& packet);
    void handleRoutingDecisionPacket(sf::Packet& packet);
    void handleBroadcastMessagePacket(sf::Packet& packet);
    void handleDropAnimationPacket(sf::Packet& packet);
    void handleRetransmissionPacket(sf::Packet& packet);

    sf::Vector2f findDeviceCoordinates(int nodeId, bool& found);
};
