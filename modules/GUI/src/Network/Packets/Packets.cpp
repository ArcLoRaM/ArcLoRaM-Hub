#include "packets.hpp"
#include <SFML/Network/Packet.hpp>
#include <string>
#include <optional>
#include <utility>
#include <iostream> // optional for debugging

// -------------------- Constructors --------------------

systemPacket::systemPacket(double distanceThreshold, std::string mode)
    : distanceThreshold(distanceThreshold), mode(std::move(mode)) {
    type = 0;
}

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

// -------------------- Packet Serialization --------------------


//IMPORTANT: We never deserialize the type of the packet using the operator <<, as it manually done through the PacketController.
//could have been avoided but I'm too lazy to change it now and It works fine if you know it.


sf::Packet& operator<<(sf::Packet& packet, const systemPacket& sp) {
    return packet << sp.type << sp.distanceThreshold << sp.mode;
}

sf::Packet& operator>>(sf::Packet& packet, systemPacket& sp) {
    return packet >>  sp.distanceThreshold >> sp.mode;
}

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

