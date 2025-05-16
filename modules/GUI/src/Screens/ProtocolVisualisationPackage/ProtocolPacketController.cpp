#include "ProtocolPacketController.hpp"
#include "../../Network/Packets/Packets.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>

void ProtocolPacketController::handlePacket(sf::Packet &packet, ProtocolVisualisationState &state, VisualiserManager &manager)
{
    int packetType;
    packet >> packetType; // Read the type first

    switch (packetType) {
    case 0: handleSystemPacket(packet, state,manager); break;
    case 1: handleTickPacket(packet, state,manager); break;
    case 2: handleStateNodePacket(packet, state,manager); break;
    case 3: handlePositionPacket(packet, state,manager); break;
    case 4: handleTransmitMessagePacket(packet, state,manager); break;
    case 5: handleReceiveMessagePacket(packet, state,manager); break;
    case 6: handleRoutingDecisionPacket(packet, state,manager); break;
    case 7: handleBroadcastMessagePacket(packet, state,manager); break;
    case 8: handleDropAnimationPacket(packet, state,manager); break;
    case 9: handleRetransmissionPacket(packet, state,manager); break;
    default:
        std::cerr << "Unknown packet type: " << packetType << std::endl;
        break;
    }


}



void ProtocolPacketController::handleSystemPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {

    //will do a proper config system later

    // systemPacket sp;
    // packet >> sp.distanceThreshold >> sp.mode;
    // DISTANCE_THRESHOLD = sp.distanceThreshold * distanceDivider;
    // COMMUNICATION_MODE = sp.mode;
    // std::string message = "Received systemPacket: distanceThreshold=" + std::to_string(sp.distanceThreshold) + ", mode=" + sp.mode;
    // {
    //     std::lock_guard<std::mutex> lock(logMutex);
    //     logMessages.push_back(message);
    // }
}

void ProtocolPacketController::handleTickPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    tickPacket tp;
    packet >> tp.tickNb;
    state.tickNumber = tp.tickNb;
}

void ProtocolPacketController::handleTransmitMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    transmitMessagePacket tmp;
    packet >> tmp.senderId >> tmp.receiverId >> tmp.isACK;

    sf::Vector2f senderCoordinates, receiverCoordinates;
    bool foundSender = false, foundReceiver = false;

    // Ask manager to find coordinates safely (manager internally locks its data)
    std::tie(senderCoordinates, foundSender) = manager.findDeviceCoordinates(tmp.senderId);
    std::tie(receiverCoordinates, foundReceiver) = manager.findDeviceCoordinates(tmp.receiverId);

    if (!foundSender || !foundReceiver) {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back("******Error: Receiver or Sender Not Found for Transmission Animation******");
    } else {
        std::unique_ptr<Arrow> arrow;
        if (tmp.isACK)
            //we should dedicate the Arrow Creation to the Manager. TODO
            arrow = std::make_unique<Arrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, config::ackArrowColor);
        else
            arrow = std::make_unique<Arrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, config::dataArrowColor);

        manager.addArrow(std::move(arrow));
    }

    state.energyExp += 20;
}



void ProtocolPacketController::handleStateNodePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    stateNodePacket snp;
    packet >> snp.nodeId >> snp.state;

    manager.updateDevicesState(snp.nodeId, snp.state);

    if (snp.state == "Communicate") {
        state.energyExp++;
    }
}

void ProtocolPacketController::handlePositionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    positionPacket pp;
    packet >> pp.nodeId >> pp.classNode >> pp.coordinates.first >> pp.coordinates.second >> pp.batteryLevel;

    pp.coordinates.first += config::horizontalOffset;
    pp.coordinates.second += config::verticalOffset;
    pp.coordinates.first *= config::distanceDivider;
    pp.coordinates.second *= config::distanceDivider;

    
        auto device = std::make_unique<Device>(pp.nodeId, pp.classNode, pp.coordinates, pp.batteryLevel);
        manager.addDevice(std::move(device));
        manager.addDeviceId(pp.nodeId);
    

    std::string message = "Received positionPacket: nodeId=" + std::to_string(pp.nodeId) + ", coordinates=(" + std::to_string(pp.coordinates.first) + ", " + std::to_string(pp.coordinates.second) + ")";
    {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back(message);
    }
}

void ProtocolPacketController::handleReceiveMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    receiveMessagePacket rmp;
    packet >> rmp.senderId >> rmp.receiverId >> rmp.state;

    sf::Vector2f senderCoordinates, receiverCoordinates;
    bool foundSender = false, foundReceiver = false;

    // Ask manager to find coordinates safely (manager internally locks its data)
    std::tie(senderCoordinates, foundSender) = manager.findDeviceCoordinates(rmp.senderId);
    std::tie(receiverCoordinates, foundReceiver) = manager.findDeviceCoordinates(rmp.receiverId);

    if (!foundSender || !foundReceiver) {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back("******Error: Receiver or Sender Not Found for Reception Icon******");
    } else {
        auto icon = std::make_unique<ReceptionIcon>(senderCoordinates, receiverCoordinates, rmp.state);
        manager.addReceptionIcon(std::move(icon));
    }

    std::string message = "Received receiveMessagePacket: senderId=" + std::to_string(rmp.senderId) + ", receiverId=" + std::to_string(rmp.receiverId) + ", state=" + rmp.state;
    {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back(message);
    }
}

void ProtocolPacketController::handleRoutingDecisionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    routingDecisionPacket rp;
    packet >> rp.receiverId >> rp.senderId >> rp.newRoute;

    {
        if (rp.newRoute) {
            manager.addRouting(rp.receiverId, rp.senderId);
        } else {
            manager.removeRouting(rp.receiverId, rp.senderId);
        }
    }

    std::string message = "Received routingDecisionPacket: receiverId=" + std::to_string(rp.receiverId) + ", senderId=" + std::to_string(rp.senderId) + ", newRoute=" + std::to_string(rp.newRoute);
    {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back(message);
    }
}

void ProtocolPacketController::handleBroadcastMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    broadcastMessagePacket bmp;
    packet >> bmp.nodeId;

    sf::Vector2f senderCoordinates;
    bool foundSender = false;
    
    std::tie(senderCoordinates, foundSender) = manager.findDeviceCoordinates(bmp.nodeId);    

    if (!foundSender) {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back("******Error: Sender Not Found for Broadcast Animation******");
    } else {
        manager.startBroadcast(senderCoordinates, config::broadcastDuration);
    }

    state.energyExp += 20;
}

void ProtocolPacketController::handleDropAnimationPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    dropAnimationPacket dap;
    packet >> dap.nodeId;

    sf::Vector2f senderCoordinates;
    bool foundSender = false;
    
    std::tie(senderCoordinates, foundSender) = manager.findDeviceCoordinates(dap.nodeId);    

    if (!foundSender) {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back("******Error: Sender Not Found for Drop Animation******");
    } else {
        auto drop = std::make_unique<PacketDrop>(senderCoordinates, 1.6f);
        manager.addDropAnimation(std::move(drop));
    }
}

void ProtocolPacketController::handleRetransmissionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    //FOr now, we only count the retransmission
    state.retransmissions++;
}
