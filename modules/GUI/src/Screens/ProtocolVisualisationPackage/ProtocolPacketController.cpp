#include "ProtocolPacketController.hpp"
#include "../../Network/Packets/Packets.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>
#include <magic_enum.hpp>
#include "../../Visualisation/Device/Device.hpp"
#include "CsvMetricWriter.hpp"

void ProtocolPacketController::handlePacket(sf::Packet &packet, ProtocolVisualisationState &state, VisualiserManager &manager)
{
    //IMPORTANT: We never deserialize the type of the packet using the operator << later in the code since we do it here.
    //This is why the operator >> and << are not symetrical in the packet definition.
    int packetType = -1; // Default to an invalid type
    packet >> packetType;

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
    case 10: handleStopSimulationPacket(packet, state,manager); break;
    default:
        std::cerr << "Unknown packet type: " << packetType << std::endl;
        break;
    }


}



void ProtocolPacketController::handleSystemPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {


    systemPacket sp;
    packet >> sp.distanceThreshold >> sp.mode;
    //missing the distance threshold. For the moment it's manually set
   state.communicationMode = sp.mode;

}

void ProtocolPacketController::handleTickPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    tickPacket tp;
    packet >> tp;
    state.tickNumber = tp.tickNb;
}

void ProtocolPacketController::handleTransmitMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    
    transmitMessagePacket tmp;
    packet >> tmp;

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


    //Metrics

    //useles?
    // state.energyExp += 20;


    if(!tmp.isACK) {
        state.totalDataPacketsSent++;//we only count the data packets sent, not the ACKs for the PDR (retransmissions are only for the data packets)
        manager.incrementPacketSent(tmp.senderId);
        manager.incrementTransmittingData(tmp.senderId);
    }
    else{
        manager.incrementTransmittingAck(tmp.senderId);    }

}

void ProtocolPacketController::handleStateNodePacket(
    sf::Packet &packet,
    ProtocolVisualisationState &state,
    VisualiserManager &manager)
{
    stateNodePacket snp;

    packet >>snp;

    std::optional<DeviceState> maybeState = magic_enum::enum_cast<DeviceState>(snp.state);
                // std::cout << "State Proposed: " << snp.state << std::endl;

    if (maybeState)
    {
        manager.updateDevicesState(snp.nodeId, *maybeState);
    }
    else
    {
        std::cerr << "Invalid state string in Protocol Packet Controller" << std::endl;
    }

    // Only increment energyExp if the optional isCommunicatingAck is true
    if (snp.state == "Communicate")
    {  
        if (snp.isCommunicatingAck.has_value())
        {
            if (snp.isCommunicatingAck.value())
            {
                //listening for ACK duration and Data duration doesn't cost the same energy
                manager.incrementListeningAck(snp.nodeId);
            }
            else
            {
                manager.incrementListeningData(snp.nodeId);
            }
        }
        else
        {
            // Optional is not set
            // (Maybe log a warning or fallback behavior)
        }
    }
}

void ProtocolPacketController::handlePositionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    positionPacket pp;
    packet >> pp;

    pp.coordinates.first += config::horizontalOffset;
    pp.coordinates.second += config::verticalOffset;
    pp.coordinates.first *= config::distanceDivider;
    pp.coordinates.second *= config::distanceDivider;


    DeviceClass deviceClass ;
    if(pp.classNode==1) deviceClass = DeviceClass::C1;
    else if(pp.classNode==2) deviceClass = DeviceClass::C2;
    else if(pp.classNode==3) deviceClass = DeviceClass::C3;
    else {
        std::cerr << "Invalid classNode in Protocol Packet Controller" << std::endl;
        return;
    }

    sf::Vector2f position(static_cast<float>(pp.coordinates.first), static_cast<float>(pp.coordinates.second));
        auto device = std::make_unique<Device>(pp.nodeId, deviceClass, position,pp.hopCount, pp.batteryLevel);
        manager.addDevice(std::move(device));
    

    std::string message = "Received positionPacket: nodeId=" + std::to_string(pp.nodeId) + ", coordinates=(" + std::to_string(pp.coordinates.first) + ", " + std::to_string(pp.coordinates.second) + ")";
    {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back(message);
    }
}

void ProtocolPacketController::handleReceiveMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    receiveMessagePacket rmp;
    packet >> rmp;

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
        if(rmp.state == "received") {
            // If the state is "received", we can assume the packet was successfully received
        }
    }

    std::string message = "Received receiveMessagePacket: senderId=" + std::to_string(rmp.senderId) + ", receiverId=" + std::to_string(rmp.receiverId) + ", state=" + rmp.state;
    {
        std::lock_guard<std::mutex> lock(state.logMutex);
        state.logMessages.push_back(message);
        
    }
}

void ProtocolPacketController::handleRoutingDecisionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    routingDecisionPacket rp;
    packet >> rp;

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
    packet >> bmp;

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
    packet >> dap;

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

    retransmissionPacket rp;
    packet >> rp;

    manager.addRetransmission(rp.nodeId);

    state.retransmissions++;

}


void ProtocolPacketController::handleStopSimulationPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager) {
    stopSimulationPacket sp;
    packet >> sp;

    
    std::lock_guard<std::mutex> lock(state.logMutex);
    state.logMessages.push_back("Simulation stopped by node: " + std::to_string(sp.nodeId));

    try {
        CsvMetricWriter writer;
        writer.writeNetworkMetricsToCsv("network_state.csv", manager, state);
        std::cout << "Network state saved to network_state.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "Error writing CSV: " << e.what() << '\n';
    }
}