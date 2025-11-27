#pragma once

#include "../../../Network/Packets/Packets.hpp"
#include "../ProtocolVisualiser.hpp"

class PacketHandler
{

public:
    PacketHandler(ProtocolVisualiser& visualiser);
    void handlePacket(sf::Packet &packet);

private:
    void handleTickPacket(sf::Packet &packet);
    void handleStateNodePacket(sf::Packet &packet);
    void handlePositionPacket(sf::Packet &packet);
    void handleTransmitMessagePacket(sf::Packet &packet);
    void handleEndTransmissionPacket(sf::Packet &packet);
    void handleReceiveMessagePacket(sf::Packet &packet);
    void handleRoutingDecisionPacket(sf::Packet &packet);
    void handleBroadcastMessagePacket(sf::Packet &packet);
    void handleDropAnimationPacket(sf::Packet &packet);
    void handleRetransmissionPacket(sf::Packet &packet);
    void handleStopSimulationPacket(sf::Packet &packet);
    void handlePongPacket(sf::Packet &packet);

    // Metrics packet handlers
    void handleNodeMetricsPacket(sf::Packet &packet);
    void handleLatencyRecordsPacket(sf::Packet &packet);
    void handleEnergySamplesPacket(sf::Packet &packet);

    ProtocolVisualiser& visualiser;
};
