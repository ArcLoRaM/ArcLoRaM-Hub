#include "packets.hpp"
#include <SFML/Network/Packet.hpp>
#include <string>
#include <optional>
#include <utility>
#include <iostream> // optional for debugging



//-------------------- TELEMETRY PACKETS --------------------

// -------------------- Constructors --------------------


tickPacket::tickPacket(int tickNb) : tickNb(tickNb) {
    type = 1;
}

stateNodePacket::stateNodePacket(int nodeId, std::string state)
    : nodeId(nodeId), state(std::move(state)) {
    type = 2;
}

positionPacket::positionPacket(int id, int classNode, std::pair<int, int> coordinates, double batteryLevel, int hopCount)
    : nodeId(id), classNode(classNode), coordinates(std::move(coordinates)), batteryLevel(batteryLevel), hopCount(hopCount) {
    type = 3;
}

transmitMessagePacket::transmitMessagePacket(int senderId, int receiverId, bool isACK)
    : senderId(senderId), receiverId(receiverId), isACK(isACK) {
    type = 4;
}
endTransmissionPacket::endTransmissionPacket(int senderId, int receiverId)
    : senderId(senderId), receiverId(receiverId) {
    type = 11;
}
receiveMessagePacket::receiveMessagePacket(int senderId, int receiverId, std::string state)
    : senderId(senderId), receiverId(receiverId), state(std::move(state)) {
    type = 5;
}

routingDecisionPacket::routingDecisionPacket(int receiverId, int senderId, bool newRoute)
    : receiverId(receiverId), senderId(senderId), newRoute(newRoute) {
    type = 6;
}

broadcastMessagePacket::broadcastMessagePacket(int nodeId) : nodeId(nodeId) {
    type = 7;
}

dropAnimationPacket::dropAnimationPacket(int nodeId) : nodeId(nodeId) {
    type = 8;
}

retransmissionPacket::retransmissionPacket(int nodeId) : nodeId(nodeId) {
    type = 9;
}

stopSimulationPacket::stopSimulationPacket(int nodeId) : nodeId(nodeId) {
    type = 10;
}

nodeMetricsPacket::nodeMetricsPacket(int tickNb, int nodeId, int sent, int acked)
    : tickNb(tickNb), nodeId(nodeId), totalPacketsSent(sent), totalAcksReceived(acked) {
    type = 12;
}

latencyRecordsPacket::latencyRecordsPacket(int tickNb, int nodeId, std::vector<LatencyRecord> records)
    : tickNb(tickNb), nodeId(nodeId), records(std::move(records)) {
    type = 13;
}

energySamplesPacket::energySamplesPacket(int tickNb, int nodeId, std::vector<std::pair<int, double>> samples)
    : tickNb(tickNb), nodeId(nodeId), samples(std::move(samples)) {
    type = 14;
}

// -------------------- Packet Serialization --------------------


//IMPORTANT: We never deserialize the field "type" of the packet using the operator >>, as it manually done through the PacketController in the GUI. The same logic would apply on this side if bidirectional communication was implemented.
//could have been avoided but I'm too lazy to change it now and It works fine if you know it.

//TODO: put const where you can =?


sf::Packet& operator<<(sf::Packet& packet, const tickPacket& tp) {
    return packet << tp.type << tp.tickNb;
}

sf::Packet& operator>>(sf::Packet& packet, tickPacket& tp) {
    return packet >>  tp.tickNb;
}

sf::Packet& operator<<(sf::Packet& packet, const stateNodePacket& snp) {

    packet << snp.type << snp.nodeId << snp.state;

    bool hasExtra = snp.isCommunicatingAck.has_value();
    packet << hasExtra;
    if (hasExtra) {
        packet << snp.isCommunicatingAck.value();
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, stateNodePacket& snp) {

    packet >>  snp.nodeId >> snp.state;

    bool hasExtra = false;
    packet >> hasExtra;
    if (hasExtra) {
        bool extra;
        packet >> extra;
        snp.isCommunicatingAck = extra;
    } else {
        snp.isCommunicatingAck.reset();
    }

    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const positionPacket& pp) {
    return packet << pp.type << pp.nodeId << pp.classNode << pp.coordinates.first << pp.coordinates.second << pp.batteryLevel << pp.hopCount;
}

sf::Packet& operator>>(sf::Packet& packet, positionPacket& pp) {
    return packet >>  pp.nodeId >> pp.classNode >> pp.coordinates.first >> pp.coordinates.second >> pp.batteryLevel >> pp.hopCount;
}

sf::Packet& operator<<(sf::Packet& packet, const transmitMessagePacket& tmp) {
    return packet << tmp.type << tmp.senderId << tmp.receiverId << tmp.isACK;
}

sf::Packet& operator>>(sf::Packet& packet, transmitMessagePacket& tmp) {
    return packet >>  tmp.senderId >> tmp.receiverId >> tmp.isACK;
}

sf::Packet &operator<<(sf::Packet &packet, const endTransmissionPacket &etp)
{
    return packet << etp.type << etp.senderId << etp.receiverId;
}

sf::Packet &operator>>(sf::Packet &packet, endTransmissionPacket &etp)
{
    return packet >> etp.senderId >> etp.receiverId;
}

sf::Packet& operator<<(sf::Packet& packet, const receiveMessagePacket& rmp) {
    return packet << rmp.type << rmp.senderId << rmp.receiverId << rmp.state;
}

sf::Packet& operator>>(sf::Packet& packet, receiveMessagePacket& rmp) {
    return packet >> rmp.senderId >> rmp.receiverId >> rmp.state;
}

sf::Packet& operator<<(sf::Packet& packet, const routingDecisionPacket& rdp) {
    return packet << rdp.type << rdp.receiverId << rdp.senderId << rdp.newRoute;
}

sf::Packet& operator>>(sf::Packet& packet, routingDecisionPacket& rdp) {
    return packet >>  rdp.receiverId >> rdp.senderId >> rdp.newRoute;
}

sf::Packet& operator<<(sf::Packet& packet, const broadcastMessagePacket& bmp) {
    return packet << bmp.type << bmp.nodeId;
}

sf::Packet& operator>>(sf::Packet& packet, broadcastMessagePacket& bmp) {
    return packet >> bmp.nodeId;
}

sf::Packet& operator<<(sf::Packet& packet, const dropAnimationPacket& dap) {
    return packet << dap.type << dap.nodeId;
}

sf::Packet& operator>>(sf::Packet& packet, dropAnimationPacket& dap) {
    return packet >>  dap.nodeId;
}

sf::Packet& operator<<(sf::Packet& packet, const retransmissionPacket& rp) {
    return packet << rp.type << rp.nodeId;
}

sf::Packet& operator>>(sf::Packet& packet, retransmissionPacket& rp) {
    return packet >>  rp.nodeId;
}


sf::Packet& operator<<(sf::Packet& packet,const stopSimulationPacket& sp) {
    return packet << sp.type<<sp.nodeId;
}

sf::Packet& operator>>(sf::Packet& packet, stopSimulationPacket& sp) {
    return packet >> sp.nodeId;
}

// nodeMetricsPacket serialization
sf::Packet& operator<<(sf::Packet& packet, const nodeMetricsPacket& nmp) {
    return packet << nmp.type << nmp.tickNb << nmp.nodeId
                  << nmp.totalPacketsSent << nmp.totalAcksReceived;
}

sf::Packet& operator>>(sf::Packet& packet, nodeMetricsPacket& nmp) {
    return packet >> nmp.tickNb >> nmp.nodeId
                  >> nmp.totalPacketsSent >> nmp.totalAcksReceived;
}

// latencyRecordsPacket serialization
sf::Packet& operator<<(sf::Packet& packet, const latencyRecordsPacket& lrp) {
    // Serialize header
    packet << lrp.type << lrp.tickNb << lrp.nodeId;

    // Serialize vector size
    packet << static_cast<int>(lrp.records.size());

    // Serialize each LatencyRecord field-by-field
    for (const auto& record : lrp.records) {
        packet << record.packetId
               << record.latency_ms
               << record.originTimestamp
               << record.deliveryTimestamp;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, latencyRecordsPacket& lrp) {
    // Deserialize header (no type)
    packet >> lrp.tickNb >> lrp.nodeId;

    // Deserialize vector size
    int size;
    packet >> size;

    // Deserialize each LatencyRecord field-by-field
    lrp.records.clear();
    lrp.records.reserve(size);
    for (int i = 0; i < size; ++i) {
        uint64_t packetId;
        int64_t latency_ms, originTimestamp, deliveryTimestamp;
        packet >> packetId >> latency_ms >> originTimestamp >> deliveryTimestamp;

        lrp.records.push_back({packetId, latency_ms, originTimestamp, deliveryTimestamp});
    }

    return packet;
}

// energySamplesPacket serialization
sf::Packet& operator<<(sf::Packet& packet, const energySamplesPacket& esp) {
    // Serialize header
    packet << esp.type << esp.tickNb << esp.nodeId;

    // Serialize vector size
    packet << static_cast<int>(esp.samples.size());

    // Serialize each pair as primitives
    for (const auto& [tick, energy] : esp.samples) {
        packet << tick << energy;
    }

    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, energySamplesPacket& esp) {
    // Deserialize header (no type)
    packet >> esp.tickNb >> esp.nodeId;

    // Deserialize vector size
    int size;
    packet >> size;

    // Deserialize each pair
    esp.samples.clear();
    esp.samples.reserve(size);
    for (int i = 0; i < size; ++i) {
        int tick;
        double energy;
        packet >> tick >> energy;
        esp.samples.emplace_back(tick, energy);
    }

    return packet;
}


//-------------------- CONTROL PACKETS ---------------------------------------------------------------------------------------------------

//------ Constructors --------------------
launchConfigCommandPacket::launchConfigCommandPacket(double threshold,  std::string topologyString,int scenarioType)
    : distanceThreshold(threshold), topologyString(std::move(topologyString)), scenarioType(scenarioType) {
    type = 100;
}

stopCommandPacket::stopCommandPacket() {
    type = 101;
}

resumeCommandPacket::resumeCommandPacket() {
    type = 102;
}

pingCommandPacket::pingCommandPacket() {
    type = 103;
}

pongPacket::pongPacket() {
    type = 104;
}

restartCommandPacket::restartCommandPacket() {
    type = 105;
}

pauseCommandPacket::pauseCommandPacket() {
    type = 106;
}

//-------------------- Packet Serialization --------------------

sf::Packet& operator<<(sf::Packet& packet, const launchConfigCommandPacket& lccp) {
    return packet << lccp.type << lccp.distanceThreshold << lccp.topologyString << lccp.scenarioType;
}
sf::Packet& operator>>(sf::Packet& packet, launchConfigCommandPacket& lccp) {
    return packet >> lccp.distanceThreshold >> lccp.topologyString >> lccp.scenarioType;
}

sf::Packet& operator<<(sf::Packet& packet, const stopCommandPacket& scp) {
    return packet << scp.type;
}

sf::Packet& operator>>(sf::Packet& packet, stopCommandPacket& scp) {
    return packet; // Nothing to deserialize
}

sf::Packet& operator<<(sf::Packet& packet, const resumeCommandPacket& rcp) {
    return packet << rcp.type;
}

sf::Packet& operator>>(sf::Packet& packet, resumeCommandPacket& rcp) {
    return packet; // Nothing to deseri alize
}

sf::Packet& operator<<(sf::Packet& packet, const pauseCommandPacket& rcp) {
    return packet << rcp.type;
}

sf::Packet& operator>>(sf::Packet& packet, pauseCommandPacket& rcp) {
    return packet; // Nothing to deserialize
}

sf::Packet& operator<<(sf::Packet& packet, const pingCommandPacket& cmd) {
    return packet << cmd.type;
}
sf::Packet& operator>>(sf::Packet& packet, pingCommandPacket& cmd) {
    return packet; // Nothing to deserialize
}

sf::Packet& operator<<(sf::Packet& packet, const restartCommandPacket& cmd) {
    return packet << cmd.type;
}
sf::Packet& operator>>(sf::Packet& packet, restartCommandPacket& cmd) {
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const pongPacket& rsp) {
    return packet << rsp.type;
}
sf::Packet& operator>>(sf::Packet& packet, pongPacket& rsp) {
    return packet;
}


