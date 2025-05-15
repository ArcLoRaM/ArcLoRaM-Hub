#include "NetworkManager.hpp"
#include "../../Visualisation/VisualiserManager/VisualiserManager.hpp"
#include "../Packets/Packets.hpp"
#include <iostream>

NetworkManager::NetworkManager(VisualiserManager& manager, sf::RenderWindow& window)
    : manager(manager),window (window),running(true), networkThread(&NetworkManager::runLoop, this) {}

NetworkManager::~NetworkManager() {
    stop();
}

void NetworkManager::stop() {
    running.store(false);
    if (networkThread.joinable()) {
        networkThread.join();
    }
}

void NetworkManager::runLoop() {
    sf::TcpListener listener;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    if (listener.listen(5000) != sf::Socket::Status::Done) {
        std::cerr << "Error starting server on port 5000\n";
        return;
    }

    listener.setBlocking(false);

    while (running.load()) {
        auto client = std::make_unique<sf::TcpSocket>();
        if (listener.accept(*client) == sf::Socket::Status::Done) {
            client->setBlocking(false);
            clients.push_back(std::move(client));
        }

        for (auto& client : clients) {
            sf::Packet packet;
            if (client->receive(packet) == sf::Socket::Status::Done) {
                processPacket(packet);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Network thread exiting cleanly\n";
}

void NetworkManager::processPacket(sf::Packet& packet) {
    int packetType;
    packet >> packetType; // Read the type first

    switch (packetType) {
    case 0: handleSystemPacket(packet); break;
    case 1: handleTickPacket(packet); break;
    case 2: handleStateNodePacket(packet); break;
    case 3: handlePositionPacket(window,packet); break;
    case 4: handleTransmitMessagePacket(packet); break;
    case 5: handleReceiveMessagePacket(packet); break;
    case 6: handleRoutingDecisionPacket(packet); break;
    case 7: handleBroadcastMessagePacket(packet); break;
    case 8: handleDropAnimationPacket(packet); break;
    case 9: handleRetransmissionPacket(packet); break;
    default:
        std::cerr << "Unknown packet type: " << packetType << std::endl;
        break;
    }
}

void NetworkManager::handleSystemPacket(sf::Packet& packet) {
    systemPacket sp;
    packet >> sp.distanceThreshold >> sp.mode;
    DISTANCE_THRESHOLD = sp.distanceThreshold * distanceDivider;
    COMMUNICATION_MODE = sp.mode;
    std::string message = "Received systemPacket: distanceThreshold=" + std::to_string(sp.distanceThreshold) + ", mode=" + sp.mode;
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(message);
    }
}

void NetworkManager::handleTickPacket(sf::Packet& packet) {
    tickPacket tp;
    packet >> tp.tickNb;
    TICK_NB = tp.tickNb;
}

// Similarly, other handlers...

void NetworkManager::handleTransmitMessagePacket(sf::Packet& packet) {
    transmitMessagePacket tmp;
    packet >> tmp.senderId >> tmp.receiverId >> tmp.isACK;

    sf::Vector2f senderCoordinates, receiverCoordinates;
    bool foundSender = false, foundReceiver = false;
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        senderCoordinates = findDeviceCoordinates(tmp.senderId, foundSender);
        receiverCoordinates = findDeviceCoordinates(tmp.receiverId, foundReceiver);
    }

    if (!foundSender || !foundReceiver) {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back("******Error: Receiver or Sender Not Found for Transmission Animation******");
    } else {
        std::unique_ptr<Arrow> arrow;
        if (tmp.isACK)
            arrow = std::make_unique<Arrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, ackArrowColor);
        else
            arrow = std::make_unique<Arrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, dataArrowColor);

        manager.addArrow(std::move(arrow));
    }

    ENERGYEXP += 20;
}

sf::Vector2f NetworkManager::findDeviceCoordinates(int nodeId, bool& found) {
    for (auto& device : manager.devices) {
        if (device->nodeId == nodeId) {
            found = true;
            return pairToVector2f(device->coordinates);
        }
    }
    found = false;
    return {};
}


void NetworkManager::handleStateNodePacket(sf::Packet& packet) {
    stateNodePacket snp;
    packet >> snp.nodeId >> snp.state;

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        for (auto& device : manager.devices) {
            if (device->nodeId == snp.nodeId) {
                device->state = snp.state;
                device->changePNG(snp.state);
            }
        }
    }

    if (snp.state == "Communicate") {
        ENERGYEXP++;
    }
}

void NetworkManager::handlePositionPacket(sf::RenderWindow &window,sf::Packet& packet) {
    positionPacket pp;
    packet >> pp.nodeId >> pp.classNode >> pp.coordinates.first >> pp.coordinates.second >> pp.batteryLevel;

    pp.coordinates.first += horizontalOffset;
    pp.coordinates.second += verticalOffset;
    pp.coordinates.first *= distanceDivider;
    pp.coordinates.second *= distanceDivider;

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        auto device = std::make_unique<Device>(window,pp.nodeId, pp.classNode, pp.coordinates, pp.batteryLevel);
        manager.addDevice(std::move(device));
        manager.addDeviceId(pp.nodeId);
    }

    std::string message = "Received positionPacket: nodeId=" + std::to_string(pp.nodeId) + ", coordinates=(" + std::to_string(pp.coordinates.first) + ", " + std::to_string(pp.coordinates.second) + ")";
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(message);
    }
}

void NetworkManager::handleReceiveMessagePacket(sf::Packet& packet) {
    receiveMessagePacket rmp;
    packet >> rmp.senderId >> rmp.receiverId >> rmp.state;

    sf::Vector2f senderPos, receiverPos;
    bool foundSender = false, foundReceiver = false;
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        senderPos = findDeviceCoordinates(rmp.senderId, foundSender);
        receiverPos = findDeviceCoordinates(rmp.receiverId, foundReceiver);
    }

    if (!foundSender || !foundReceiver) {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back("******Error: Receiver or Sender Not Found for Reception Icon******");
    } else {
        auto icon = std::make_unique<ReceptionIcon>(senderPos, receiverPos, rmp.state);
        manager.addReceptionIcon(std::move(icon));
    }

    std::string message = "Received receiveMessagePacket: senderId=" + std::to_string(rmp.senderId) + ", receiverId=" + std::to_string(rmp.receiverId) + ", state=" + rmp.state;
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(message);
    }
}

void NetworkManager::handleRoutingDecisionPacket(sf::Packet& packet) {
    routingDecisionPacket rp;
    packet >> rp.receiverId >> rp.senderId >> rp.newRoute;

    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        if (rp.newRoute) {
            manager.addRouting(rp.receiverId, rp.senderId);
        } else {
            manager.removeRouting(rp.receiverId, rp.senderId);
        }
    }

    std::string message = "Received routingDecisionPacket: receiverId=" + std::to_string(rp.receiverId) + ", senderId=" + std::to_string(rp.senderId) + ", newRoute=" + std::to_string(rp.newRoute);
    {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back(message);
    }
}

void NetworkManager::handleBroadcastMessagePacket(sf::Packet& packet) {
    broadcastMessagePacket bmp;
    packet >> bmp.nodeId;

    sf::Vector2f senderCoordinates;
    bool foundSender = false;
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        senderCoordinates = findDeviceCoordinates(bmp.nodeId, foundSender);
    }

    if (!foundSender) {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back("******Error: Sender Not Found for Broadcast Animation******");
    } else {
        manager.startBroadcast(senderCoordinates, broadcastDuration);
    }

    ENERGYEXP += 20;
}

void NetworkManager::handleDropAnimationPacket(sf::Packet& packet) {
    dropAnimationPacket dap;
    packet >> dap.nodeId;

    sf::Vector2f senderCoordinates;
    bool foundSender = false;
    {
        std::lock_guard<std::mutex> lock(deviceMutex);
        senderCoordinates = findDeviceCoordinates(dap.nodeId, foundSender);
    }

    if (!foundSender) {
        std::lock_guard<std::mutex> lock(logMutex);
        logMessages.push_back("******Error: Sender Not Found for Drop Animation******");
    } else {
        auto drop = std::make_unique<PacketDrop>(senderCoordinates, 1.6f);
        manager.addDropAnimation(std::move(drop));
    }
}

void NetworkManager::handleRetransmissionPacket(sf::Packet& packet) {
    //FOr now, we only count the retransmission
    NBRETRANSMISSION++;
}
