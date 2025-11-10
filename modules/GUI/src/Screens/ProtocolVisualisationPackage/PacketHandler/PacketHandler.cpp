#include "PacketHandler.hpp"
#include "../../Network/Packets/Packets.hpp"
#include "../../Shared/Config.hpp"
#include <iostream>
#include <magic_enum.hpp>
#include "../../Objects/Device/Device.hpp"
#include "../Metrics/IO/CsvMetricWriter.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"

PacketHandler::PacketHandler(ProtocolVisualiser& visualiser)
: visualiser(visualiser)
{
}

void PacketHandler::handlePacket(sf::Packet &packet)
{
    //IMPORTANT: We never deserialize the type of the packet using the operator << later in the code since we do it here.
    //This is why the operator >> and << are not symetrical in the packet definition.
    int packetType = -1; // Default to an invalid type
    packet >> packetType;

    switch (packetType) {

    //Protocol actions
    case 0: handleSystemPacket(packet); break;
    case 1: handleTickPacket(packet); break;
    case 2: handleStateNodePacket(packet); break;
    case 3: handlePositionPacket(packet); break;
    case 4: handleTransmitMessagePacket(packet); break;
    case 5: handleReceiveMessagePacket(packet); break;
    case 6: handleRoutingDecisionPacket(packet); break;
    case 7: handleBroadcastMessagePacket(packet); break;
    case 8: handleDropAnimationPacket(packet); break;
    case 9: handleRetransmissionPacket(packet); break;
    case 10: handleStopSimulationPacket(packet); break;
    case 11: handleEndTransmissionPacket(packet); break;

    //server - client 
    case 104: handlePongPacket(packet);break;
    
    default:
        std::cerr << "Unknown packet type: " << packetType << std::endl;
        break;
    }


}

void PacketHandler::handlePongPacket(sf::Packet &packet)
{
    TcpServer::instance().updateLastPong();
    
}


void PacketHandler::handleSystemPacket(sf::Packet& packet) {

    //old serialization way
    systemPacket sp;
    packet >> sp.distanceThreshold >> sp.mode;
    //missing the distance threshold. For the moment it's manually set
   visualiser.getProtocolState().communicationMode = sp.mode;

}

void PacketHandler::handleTickPacket(sf::Packet& packet) {
    tickPacket tp;
    packet >> tp;
    visualiser.getProtocolState().tickNumber = tp.tickNb;
}

void PacketHandler::handleTransmitMessagePacket(sf::Packet& packet) {
    
    transmitMessagePacket tmp;
    packet >> tmp;

    sf::Vector2f senderCoordinates, receiverCoordinates;
    bool foundSender = false, foundReceiver = false;

    // Ask manager to find coordinates safely (manager internally locks its data)
    std::optional<std::pair<sf::Vector2f, sf::Vector2f>> positions = visualiser.getDeviceManager().getDevicesPosition(tmp.senderId,tmp.receiverId);

    if (!positions.has_value()) {

        //TODO: make a log for the UI, and a log for the simulator
        std::cerr << "******Error: Receiver or Sender Not Found for Transmission Arrow******" << std::endl;

    } else {
        senderCoordinates = positions->first;
        receiverCoordinates = positions->second;
        std::unique_ptr<BroadcastArrow> arrow;

        if (tmp.isACK)
            arrow = std::make_unique<BroadcastArrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, config::ackArrowColor);
        else
            arrow = std::make_unique<BroadcastArrow>(senderCoordinates, receiverCoordinates, tmp.senderId, tmp.receiverId, config::dataArrowColor);

        visualiser.getAnimationManager().addBroadcastArrow(std::move(arrow));
    }

    //Old metric system, erase if confirmed outdated
    // if(!tmp.isACK) {
    //     state.totalDataPacketsSent++;//we only count the data packets sent, not the ACKs for the PDR (retransmissions are only for the data packets)
    //     manager.incrementPacketSent(tmp.senderId);
    //     manager.incrementTransmittingData(tmp.senderId);
    // }
    // else{
    //     manager.incrementTransmittingAck(tmp.senderId);    }

}

void PacketHandler::handleEndTransmissionPacket(sf::Packet &packet)
{
    endTransmissionPacket etp;
    packet >> etp;
    visualiser.getAnimationManager().removeBroadcastArrow(etp.senderId, etp.receiverId);

}

void PacketHandler::handleStateNodePacket(sf::Packet &packet)
{
    stateNodePacket snp;

    packet >>snp;

    std::optional<DeviceState> maybeState = magic_enum::enum_cast<DeviceState>(snp.state);
                // std::cout << "State Proposed: " << snp.state << std::endl;

    if (maybeState)
    {
        visualiser.getDeviceManager().updateDeviceState(snp.nodeId, *maybeState);
    }
    else
    {
        std::cerr << "Invalid state string in Protocol Packet Controller" << std::endl;
    }

    //OLD Metric system, erase if irrelevant TODO
    // // Only increment energyExp if the optional isCommunicatingAck is true
    // if (snp.state == "Communicate")
    // {  
    //     if (snp.isCommunicatingAck.has_value())
    //     {
    //         if (snp.isCommunicatingAck.value())
    //         {
    //             //listening for ACK duration and Data duration doesn't cost the same energy
    //             manager.incrementListeningAck(snp.nodeId);
    //         }
    //         else
    //         {
    //             manager.incrementListeningData(snp.nodeId);
    //         }
    //     }
    //     else
    //     {
    //         // Optional is not set
    //         // (Maybe log a warning or fallback behavior)
    //     }
    // }
}

void PacketHandler::handlePositionPacket(sf::Packet& packet) {
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
        visualiser.getDeviceManager().addDevice(std::move(device));
    

    std::string message = "Received positionPacket: nodeId=" + std::to_string(pp.nodeId) + ", coordinates=(" + std::to_string(pp.coordinates.first) + ", " + std::to_string(pp.coordinates.second) + ")";
    {
        std::lock_guard<std::mutex> lock(visualiser.getProtocolState().logMutex);
        visualiser.getProtocolState().logMessages.push_back(message);
    }
}

void PacketHandler::handleReceiveMessagePacket(sf::Packet& packet) {
    receiveMessagePacket rmp;
    packet >> rmp;

    sf::Vector2f senderCoordinates, receiverCoordinates;
    bool foundSender = false, foundReceiver = false;

    // Ask manager to find coordinates safely (manager internally locks its data)
    std::optional<std::pair<sf::Vector2f, sf::Vector2f>> positions = visualiser.getDeviceManager().getDevicesPosition(rmp.senderId, rmp.receiverId);


    if (!positions.has_value()) {

        std::cerr << "******Error: Receiver or Sender Not Found for Reception Animation******" << std::endl;
    } else {
        auto maybeReceptionState = magic_enum::enum_cast<ReceptionState>(rmp.state);
        if (!maybeReceptionState) {
            std::cerr << "Invalid reception state in receiveMessagePacket: " << rmp.state << std::endl;
            return;
        }
        ReceptionState receptionState = *maybeReceptionState;
        auto icon = std::make_unique<ReceptionIcon>(positions->first, positions->second, receptionState);
        visualiser.getAnimationManager().addReceptionIcon(std::move(icon));
    }

    // std::string message = "Received receiveMessagePacket: senderId=" + std::to_string(rmp.senderId) + ", receiverId=" + std::to_string(rmp.receiverId) + ", state=" + rmp.state;
    // {
    //     std::lock_guard<std::mutex> lock(visualiser.getProtocolState().logMutex);
    //     visualiser.getProtocolState().logMessages.push_back(message);

    // }
}

void PacketHandler::handleRoutingDecisionPacket(sf::Packet& packet) {
    routingDecisionPacket rp;
    packet >> rp;

    {
        if (rp.newRoute) {
            visualiser.getRoutingManager().addRouting(rp.receiverId, rp.senderId);
        } else {
            visualiser.getRoutingManager().removeRouting(rp.receiverId, rp.senderId);
        }
    }

    std::string message = "Received routingDecisionPacket: receiverId=" + std::to_string(rp.receiverId) + ", senderId=" + std::to_string(rp.senderId) + ", newRoute=" + std::to_string(rp.newRoute);
    {
        std::lock_guard<std::mutex> lock(visualiser.getProtocolState().logMutex);
        visualiser.getProtocolState().logMessages.push_back(message);
    }
}

void PacketHandler::handleBroadcastMessagePacket(sf::Packet& packet) {
//     broadcastMessagePacket bmp;
//     packet >> bmp;

//    std::optional<sf::Vector2f> senderCoordinates = visualiser.getDeviceManager().getDevicePosition(bmp.nodeId);

//     if (!senderCoordinates.has_value()) {
//         std::lock_guard<std::mutex> lock(visualiser.getProtocolState().logMutex);
//         visualiser.getProtocolState().logMessages.push_back("******Error: Sender Not Found for Broadcast Animation******");
//         std::cerr << "******Error: Sender Not Found for Broadcast Animation******" << std::endl;
//     } else {
//         visualiser.getAnimationManager().addBroadcast(senderCoordinates.value(), config::broadcastDuration);
//     }

//     visualiser.getProtocolState().energyExp += 20;


std::cerr << "BroadcastMessagePacket handling is currently disabled." << std::endl;
}

void PacketHandler::handleDropAnimationPacket(sf::Packet& packet) {
    dropAnimationPacket dap;
    packet >> dap;

   std::optional<sf::Vector2f> senderCoordinates = visualiser.getDeviceManager().getDevicePosition(dap.nodeId);

    if (!senderCoordinates.has_value()) {
        std::lock_guard<std::mutex> lock(visualiser.getProtocolState().logMutex);
        visualiser.getProtocolState().logMessages.push_back("******Error: Sender Not Found for Drop Animation******");
    } else {
        auto drop = std::make_unique<PacketDrop>(senderCoordinates.value(), 1.6f);
        visualiser.getAnimationManager().addDropAnimation(std::move(drop));
    }
}

void PacketHandler::handleRetransmissionPacket(sf::Packet& packet) {


    //OLD METRIC SYSTEM: TODO REMOVE


    // For now, we only count the retransmission

    // retransmissionPacket rp;
    // packet >> rp;

    // manager.addRetransmission(rp.nodeId);

    // state.retransmissions++;

}


void PacketHandler::handleStopSimulationPacket(sf::Packet& packet) {


    //OLD CONDITIONAL STOP SYSTEM, maybe removed later TODO

    // stopSimulationPacket sp;
    // packet >> sp;

    
    // std::lock_guard<std::mutex> lock(state.logMutex);
    // state.logMessages.push_back("Simulation stopped by node: " + std::to_string(sp.nodeId));

    // try {
    //     CsvMetricWriter writer;
    //     writer.writeNetworkMetricsToCsv("output/network_state.csv", manager, state);
    //     std::cout << "Network state saved to network_state.csv\n";
    // } catch (const std::exception& e) {
    //     std::cerr << "Error writing CSV: " << e.what() << '\n';
    // }

}