#include "C3_RRC_UplinkHandler.hpp"
#include "../../../Miscellaneous/Utilities/Utilities.hpp"

using namespace common;
using namespace packet_tool;

void C3_RRC_UplinkHandler::handleCommunication(C3_Node &node)
{
    if (node.currentState == NodeState::Transmitting)
        if (node.RRC_UPLINK_shouldReplyACK)
        {

            buildAndTransmitAckPacket(node);
            node.RRC_UPLINK_shouldReplyACK = false;
            node.logEvent("TxACK...");
        }
}

void C3_RRC_UplinkHandler::buildAndTransmitAckPacket(C3_Node &node)
{
    std::vector<uint8_t> ackPacket;

    // preallocate the space for optimization
    ackPacket.reserve(field_sizes::type +
                      field_sizes::global_id + // Sender
                      field_sizes::global_id + // receiver
                      rrc_uplink::localIDPacketBytesSize +
                      field_sizes::hash);

    // prepare the fields:
    std::vector<uint8_t> senderGlobalId = decimalToBytes(node.nodeId, field_sizes::global_id);                   // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> receiverGlobalId = decimalToBytes(node.RRC_UPLINK_lastSenderId, field_sizes::global_id);           // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> localIDPacket = decimalToBytes(node.RRC_UPLINK_lastLocalIDPacket, common::localIDPacketBytesSize); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00};                                                // Hash Function is 4 byte long in the simulation AND in real life

    // Append all fields
    appendVector(ackPacket, std::vector<uint8_t>{static_cast<uint8_t>(common::PacketType::RRC_Uplink_ACK)});
    appendVector(ackPacket, senderGlobalId);
    appendVector(ackPacket, receiverGlobalId);
    appendVector(ackPacket, localIDPacket);
    appendVector(ackPacket, hashFunction);

    node.addMessageToTransmit(ackPacket, rrc_uplink::timeOnAirAckPacket_ms);
    node.adressedPacketTransmissionDisplay(node.RRC_UPLINK_lastSenderId, true);
}

bool C3_RRC_UplinkHandler::receiveMessage(C3_Node &node, const std::vector<uint8_t> &message)
{

    // todo: could be refactored using the display functions from base ndoe class.
    // Node must listen/communicate and not ransmit  to receive a message
    if (!canNodeReceiveMessage(node))
    {

        uint16_t senderId = extractBytesFromField(message, field_names::senderGlobalId, rrc_uplink_data_fields);

        node.receptionStateDisplay(senderId,ReceptionState::NotListening    );
        return false;
    }

    PacketType pktType = static_cast<PacketType>(message[0]);
    if (pktType != PacketType::RRC_Uplink_Data)
    {
        // not a data packet, we don't care

        node.dropAnimationDisplay();
        return false;
    }

    // We received a data packet, if it's relevant for us, we should send an ack
    uint16_t receiverId = extractBytesFromField(message, field_names::receiverGlobalId, rrc_uplink_data_fields);

    if (receiverId != node.nodeId)
    {
        // not for us, we don't care

        node.dropAnimationDisplay();
        return false;
    }

    // we received a packet for us, we should send an ack no matter what happened before (ack can be lost)
    node.RRC_UPLINK_shouldReplyACK = true;
    node.logEvent("RcvDATA will ACK");
    // we keep a memory of the packets received with their sender ID and local ID packet (specific to a link).
    node.RRC_UPLINK_lastSenderId = extractBytesFromField(message, field_names::senderGlobalId, rrc_uplink_data_fields);
    node.RRC_UPLINK_lastLocalIDPacket = extractBytesFromField(message, field_names::localIDPacket, rrc_uplink_data_fields);
    node.RRC_UPLINK_receivedPacketsId[node.RRC_UPLINK_lastSenderId].insert(node.RRC_UPLINK_lastLocalIDPacket); // duplicates are automatically ignored

    int totalCount = 0;

    for (const auto &pair : node.RRC_UPLINK_receivedPacketsId)
    {
        totalCount += pair.second.size();
    }

    // this was a stop simulation mechanism present at some point for the first publication
    // not necessary anymore

    // //we have only one C3 node, so we can stop the simulation if we received enough packets
    // //if multiple C3, a static variable for Class C3 node should be used to count the total packets received
    // if(totalCount>=numberPacketsReceivedByC3ToStopSimulation){

    //     sf::Packet stopPacketReceiver;
    //     stopSimulationPacket stopPacket(nodeId);
    //     stopPacketReceiver<<stopPacket;
    //     logger.sendTcpPacket(stopPacketReceiver);

    // }
    node.receptionStateDisplay(node.RRC_UPLINK_lastSenderId, ReceptionState::Received);

    // We don't really care about the payload and the hash function at this stage of development

    return true;
}

bool C3_RRC_UplinkHandler::canNodeReceiveMessage(const C3_Node &node) const
{
    // State Condition: node must be listening to receive a message
    if (node.getCurrentState() != NodeState::Listening)
    {
        return false;
    }
    return true;
}

bool C3_RRC_UplinkHandler::canTransmitFromSleeping(C3_Node &node)
{
    node.setCurrentState(NodeState::Transmitting);
    node.logEvent("WakeUp:Trsmt");
    node.nodeStateDisplay("Transmit", std::nullopt);
    return true;
}

bool C3_RRC_UplinkHandler::canListenFromSleeping(C3_Node &node)
{
    node.setCurrentState(NodeState::Listening);
    node.nodeStateDisplay("Listen", std::nullopt);
    node.logEvent("WakeUp:Lstn");
    return true;
}

bool C3_RRC_UplinkHandler::canSleepFromListening(C3_Node &node)
{
    node.setCurrentState(NodeState::Sleeping);
    node.nodeStateDisplay("Sleep", std::nullopt);
    node.logEvent("FallAsleep");
    return true;
}

bool C3_RRC_UplinkHandler::canSleepFromTransmitting(C3_Node &node)
{
    node.setCurrentState(NodeState::Sleeping);
    node.nodeStateDisplay("Sleep", std::nullopt);
    node.logEvent("FallAsleep");
    return true;
}







// unauthorized transitions in this mode for this class
bool C3_RRC_UplinkHandler::canSleepFromSleeping(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canSleepFromCommunicating(C3_Node &node)
{
    return false;
}
bool C3_RRC_UplinkHandler::canCommunicateFromTransmitting(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canCommunicateFromListening(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canCommunicateFromSleeping(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canTransmitFromListening(C3_Node &node)
{
    return false;
}
bool C3_RRC_UplinkHandler::canTransmitFromTransmitting(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canTransmitFromCommunicating(C3_Node &node)
{
    return false;
}
bool C3_RRC_UplinkHandler::canListenFromTransmitting(C3_Node &node)
{
    return false;
}
bool C3_RRC_UplinkHandler::canListenFromListening(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canListenFromCommunicating(C3_Node &node)
{
    return false;
}

bool C3_RRC_UplinkHandler::canCommunicateFromCommunicating(C3_Node &node)
{
    return false;
}