#include "C2_RRC_UplinkHandler.hpp"
#include "../../../Setup/Common.hpp"
#include "../../../Miscellaneous/Utilities/Utilities.hpp"
#include "C2_RRC_UplinkSlotManager.hpp"
#include "../../../Metrics/MetricsAggregator.hpp"
#include "../../../Metrics/NodeMetrics.hpp"

using namespace packet_tool;
using namespace common;
bool C2_RRC_UplinkHandler::receiveMessage(C2_Node &node, const std::vector<uint8_t> &message)
{
     if (message.empty()) {
        throw std::runtime_error("Received empty packet");
    }
    
    //assume type is always placed at first position
    common::PacketType pktType = static_cast<common::PacketType>(message[0]);
    uint16_t senderId;
    uint16_t receiverId;
    if(pktType==common::PacketType::RRC_Uplink_Data)
    {
        senderId = extractBytesFromField(message, field_names::senderGlobalId, common::rrc_uplink_data_fields);
        receiverId = extractBytesFromField(message, field_names::receiverGlobalId, common::rrc_uplink_data_fields);

    }
    else if(pktType==common::PacketType::RRC_Uplink_ACK)
    {
        senderId = extractBytesFromField(message, field_names::senderGlobalId, common::rrc_uplink_ack_fields);
        receiverId = extractBytesFromField(message, field_names::receiverGlobalId, common::rrc_uplink_ack_fields);
    }
    
    if (!canNodeReceiveMessage(node))
    {
        node.receptionStateDisplay(senderId, ReceptionState::NotListening);
        return false;
    }

    
    if(pktType!=common::PacketType::RRC_Uplink_Data && pktType!=common::PacketType::RRC_Uplink_ACK){
        node.dropAnimationDisplay();
        return false;
    }

    // if packet is not for this node,message is dropped
    if (receiverId != node.nodeId)
    {
        node.dropAnimationDisplay();
        return false;
    }

    // is it a data packet and  are we in a data window
    if (pktType==common::PacketType::RRC_Uplink_Data && !node.RRC_UPLINK_isACKSlot)
     {   
        node.logEvent("RecvData, will ACK");
        // we received a packet for us, we should send an ack no matter what happened before (ack can be lost so we should not check if we already sent one)
        auto lastLocalIdPacket = extractBytesFromField(message, field_names::localIDPacket, common::rrc_uplink_data_fields);
        auto lastSenderId = extractBytesFromField(message, field_names::senderGlobalId, common::rrc_uplink_data_fields);
        node.RRC_UPLINK_ackInformation->setNewAckInformation(lastSenderId, lastLocalIdPacket);
        handleDataPacketReception(node,message, lastSenderId, lastLocalIdPacket);
        // Indicate the visualiser the packet is received and handled
        node.receptionStateDisplay(lastSenderId, ReceptionState::Received);
        
    }
    else if (pktType == common::PacketType::RRC_Uplink_ACK && node.RRC_UPLINK_isACKSlot)
    {
        // it's an ACK packet and we are in an ACK window
        uint16_t localIdPacket = extractBytesFromField(message, field_names::localIDPacket, common::rrc_uplink_ack_fields);
        uint16_t senderId = extractBytesFromField(message, field_names::senderGlobalId, common::rrc_uplink_ack_fields);
        // Visualiser Display behavior inside the below function.
        handleAckPacketReception(node,senderId, localIdPacket);
        node.logEvent("RecvACK");
    }

    return true;
}

void C2_RRC_UplinkHandler::handleDataPacketReception(C2_Node &node,const std::vector<uint8_t> &message, uint16_t senderId, uint32_t packetId)
{

    // we store the local packet ID in map. This is how we know if the packet is new or not
    auto &packetList = node.RRC_UPLINK_packetsMap[senderId]; // Get the vector for the sender
    // Only add if the packet is not already present
    if (std::find(packetList.begin(), packetList.end(), packetId) == packetList.end())
    {
        packetList.push_back(packetId);
        // TODO: it's a simulation, in real implementation, we should save the message and add it to a buffer
        node.RRC_UPLINK_nbPayloadLeft++;

        // === METRICS: Track received packet for forwarding ===
        if (node.metricsAggregator) {
            // Look up the globalId of the received packet
            GlobalPacketID receivedGlobalId = node.metricsAggregator->lookupGlobalPacketId(
                senderId,
                packetId
            );

            if (receivedGlobalId != 0) {
                // Add to forwarding queue so we know to use recordPacketForwarded when transmitting
                node.RRC_UPLINK_forwardingQueue.push(receivedGlobalId);
            }
        }
    }
}

void C2_RRC_UplinkHandler::handleAckPacketReception(C2_Node &node,uint16_t senderId, uint32_t packetId)
{
    // is it the Ack packet we were waiting for?
    if (packetId == node.RRC_UPLINK_localIDPacketCounter)
    {
        // === METRICS: Track ACK reception ===
        if (node.metricsAggregator && node.metrics) {
            // Lookup the global packet ID from (nodeId, localId)
            GlobalPacketID globalId = node.metricsAggregator->lookupGlobalPacketId(
                node.getId(),
                packetId
            );

            if (globalId != 0) {  // 0 means not found
                // Record ACK reception for PDR tracking
                node.metrics->recordAckReception(globalId, node.getCurrentTime());
            }
        }

        // we received the ACK for the last packet we sent
        node.RRC_UPLINK_nbPayloadLeft--;        // todo:in real life, we remove the payload from the buffer
        node.RRC_UPLINK_localIDPacketCounter++; // increasing the counter to indicate we send a "new" packet
        node.RRC_UPLINK_retransmissionCounterHelper->setIsExpectingAck(false);
        node.receptionStateDisplay(senderId, ReceptionState::Received);
    }
    else
    {
        node.dropAnimationDisplay();
    }
}

bool C2_RRC_UplinkHandler::canNodeReceiveMessage(const C2_Node &node) const
{
    NodeState currentStatelocal = node.getCurrentState();

    // State Condition: node must be listening to receive a message
    if (currentStatelocal != NodeState::Listening && currentStatelocal != NodeState::Communicating)
    {
        return false;
    }

    // They also need to NOT be actively transmitting
    else if (currentStatelocal == NodeState::Communicating)
    {
        return !node.RRC_UPLINK_isTransmittingWhileCommunicating;
    }
    return true;
}

// 🔽 Sample transition rules. Replace with your real logic.
bool C2_RRC_UplinkHandler::canTransmitFromListening(C2_Node &) { return true; }
bool C2_RRC_UplinkHandler::canTransmitFromSleeping(C2_Node &) { return false; }
bool C2_RRC_UplinkHandler::canTransmitFromTransmitting(C2_Node &) { return false; }
bool C2_RRC_UplinkHandler::canTransmitFromCommunicating(C2_Node &) { return true; }

bool C2_RRC_UplinkHandler::canListenFromTransmitting(C2_Node &) { return false; }
bool C2_RRC_UplinkHandler::canListenFromSleeping(C2_Node &) { return true; }
bool C2_RRC_UplinkHandler::canListenFromListening(C2_Node &) { return true; }
bool C2_RRC_UplinkHandler::canListenFromCommunicating(C2_Node &) { return true; }

bool C2_RRC_UplinkHandler::canSleepFromTransmitting(C2_Node &) { return false; }
bool C2_RRC_UplinkHandler::canSleepFromListening(C2_Node &) { return true; }
bool C2_RRC_UplinkHandler::canSleepFromSleeping(C2_Node &node) { 
    node.RRC_UPLINK_retransmissionCounterHelper->toggleSecondSleepWindow();
    return true;
 }
bool C2_RRC_UplinkHandler::canSleepFromCommunicating(C2_Node &node) { 
    
     node.RRC_UPLINK_retransmissionCounterHelper->toggleSecondSleepWindow();

    if (node.RRC_UPLINK_retransmissionCounterHelper->getIsExpectingAck() && node.RRC_UPLINK_retransmissionCounterHelper->getSecondSleepWindow())
    {
        // we sent a Data packet and didn't receive an ACK in the next transmission window, so there will be retransmission-> specific metric in the vizualiser.
        retransmissionPacket retransmissionPacket(node.nodeId);
        sf::Packet retransmissionPacketReceiver;
        retransmissionPacketReceiver << retransmissionPacket;
        node.logger.sendTcpPacket(retransmissionPacketReceiver);
        node.RRC_UPLINK_retransmissionCounterHelper->setIsExpectingAck(false);
        node.logEvent("NoACKReceived...");
    }

    node.setCurrentState(NodeState::Sleeping);
    node.logEvent("FallAsleep");
    node.nodeStateDisplay("Sleep", std::nullopt);
    return true;
}

bool C2_RRC_UplinkHandler::canCommunicateFromTransmitting(C2_Node &) { return false; }
bool C2_RRC_UplinkHandler::canCommunicateFromListening(C2_Node &) { return true; }
bool C2_RRC_UplinkHandler::canCommunicateFromSleeping(C2_Node &node) {
    // Todo: should be put into the constructor but doesn´t work, probably an optionnal not being initialized?
    //  the first state transition, we display rooting in the visualiser if applicable
    
        if (!node.RRC_UPLINK_routingDisplayed)
        {
            node.RRC_UPLINK_displayRouting();
            node.RRC_UPLINK_routingDisplayed = true;
        }
    

    node.RRC_UPLINK_isACKSlot = !node.RRC_UPLINK_isACKSlot; // switch to new slot category everytime we enter a new communication window (Data or ACK slot)


    node.RRC_UPLINK_isTransmittingWhileCommunicating = false; // used to detect when node cannot physically receive messages because they transmit

    if (!node.RRC_UPLINK_isACKSlot) // Advance the global data slot category (mod 3) (part of the protocol) for the next iteration
        node.RRC_UPLINK_currentDataSlotCategory = ((node.RRC_UPLINK_currentDataSlotCategory + 1) % 3);

    // Change of state is allowed if the current DataSlotCategory is corresponding to the HopCountmodulo (the node might transmit info)
    // or the HopCCountmodulo+1 (the node might receive ACK)
    bool showDisplay = true;
    if (node.RRC_UPLINK_currentDataSlotCategory != node.RRC_UPLINK_fixedSlotCategory && node.RRC_UPLINK_currentDataSlotCategory != ((node.RRC_UPLINK_fixedSlotCategory + 1) % 3))
    {
         node.logEvent("Asleep");
        return true; //returning true makes sense here as the node do not wake up but it's part of the PROTOCOL
    }

    // The node is allowed to wake up according to the TDMA (see previous)
    // However, It will only do so:
    //- if it's an ack window and he's expecting an ack or has to send one after receiving a data packet
    //- if it's a data window, it has data to transmit and it has chosen (randomly) to transmit for this window (channel access strategy)

    
    // ACK
    if (node.RRC_UPLINK_isACKSlot)
    {
        if (node.RRC_UPLINK_ackInformation->shouldReplyAck())
        {
            node.setCurrentState(NodeState::Communicating);
            node.nodeStateDisplay("Communicate", true);
            node.logEvent("Wakeup:TxACK");
            return true;
        }

        else if (node.RRC_UPLINK_retransmissionCounterHelper->getIsExpectingAck())
        {
            node.setCurrentState(NodeState::Communicating);
            node.nodeStateDisplay("Communicate", true);
            // the node is expecting an ACK, so it will wake up to listen
            // logger.logMessage(Log("Node " + std::to_string(nodeId) + " is expecting an ACK, will wake up to listen", true));
            node.logEvent("Wakeup:RxACK");
            return true;
        }
        node.logEvent("ACKNoAction");
        return true;
    }
    else
    {
        // Node can only act when its fixed category matches the current simulation slot category
        if (node.RRC_UPLINK_fixedSlotCategory == node.RRC_UPLINK_currentDataSlotCategory)
        {
            // Refill slots if we've exhausted them but still have payloads to send
            if (!node.RRC_UPLINK_slotManager->hasSlots() && node.RRC_UPLINK_nbPayloadLeft > 0)
            {
                node.RRC_UPLINK_slotManager->initializeRandomSlots(common::maxNodeSlots, common::totalNumberOfSlotsPerModuloNode);
                node.logEvent("RefillSlots");
            }

            // the node has the opportunity to transmit data
            if (node.RRC_UPLINK_slotManager->canTransmitNow() && node.RRC_UPLINK_nbPayloadLeft > 0)
            {
                node.RRC_UPLINK_slotManager->consumeSlot();
                // we allow the state transition as the node will be sending data
                node.setCurrentState(NodeState::Communicating);
                node.logEvent("Wakeup:TxDATA");
                node.nodeStateDisplay("Communicate", false);
                node.RRC_UPLINK_slotManager->decrementAllSlots();//probably stupid computation wise but this is only temporary as soon node will dynamically determine their schedule, not statically
                return true;
            }
            node.RRC_UPLINK_slotManager->decrementAllSlots();
        }
        else
        {
            // the node might receive a Data Packet. It must be listening
            //TODO: makes more sense to actually set the state to listening here,
            //but this require to change the scheduling during provisionning...
            node.setCurrentState(NodeState::Communicating);
            node.nodeStateDisplay("Communicate", false);
            node.logEvent("Wakeup:RxDATA");
            return true;

        }

        node.logEvent("DATANoAction");
        return true;
    }

}

bool C2_RRC_UplinkHandler::canCommunicateFromCommunicating(C2_Node &) { return true; }

void C2_RRC_UplinkHandler::handleCommunication(C2_Node &node)
{

    if (node.currentState == NodeState::Communicating)
    {

        if (!node.RRC_UPLINK_isACKSlot && node.RRC_UPLINK_fixedSlotCategory == node.RRC_UPLINK_currentDataSlotCategory) // if its an DATA slot and we are awake, it means we have to transmit
        {

            if (node.RRC_UPLINK_infoFromBeaconPhase)
            {
                node.RRC_UPLINK_isTransmittingWhileCommunicating = true;
                buildAndTransmitDataPacket(node, {0xFF, 0xFF, 0xFF, 0xFF});
                node.RRC_UPLINK_retransmissionCounterHelper->setIsExpectingAck(true);
                node.logEvent("Tx Data..");

                node.adressedPacketTransmissionDisplay(node.RRC_UPLINK_infoFromBeaconPhase->getNextNodeIdInPath(), false);
            }
            else
            {
                node.logEvent("No Routing info, cannot transmit data");
            }
        }

        // otherwise, its an ack slot, we transmit an ACK only if we received a message before
        else
        {
            if (node.RRC_UPLINK_ackInformation->shouldReplyAck())
            {
                node.RRC_UPLINK_isTransmittingWhileCommunicating = true;
                buildAndTransmitAckPacket(node);
                node.logEvent("Tx ACK..");
            }
        }
    }
}

void C2_RRC_UplinkHandler::buildAndTransmitDataPacket(C2_Node &node, std::vector<uint8_t> payload)
{

    // create the data packet
    std::vector<uint8_t> dataPacket;

    // preallocate the space for optimization
    dataPacket.reserve(common::field_sizes::type +
                       common::field_sizes::global_id + // Sender
                       common::field_sizes::global_id + // receiver
                       common::rrc_uplink::localIDPacketBytesSize +
                       common::rrc_uplink::payloadSizeBytesSize +
                       common::field_sizes::hash);

    // prepare the fields
    std::vector<uint8_t> senderGlobalId = decimalToBytes(node.nodeId, common::field_sizes::global_id);                                       // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> receiverGlobalId = decimalToBytes(node.RRC_UPLINK_infoFromBeaconPhase->getNextNodeIdInPath(), common::field_sizes::global_id); // Receiver Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> localIDPacket = decimalToBytes(node.RRC_UPLINK_localIDPacketCounter, common::rrc_uplink::localIDPacketBytesSize);              // we increase the counter if we receive the ACK

    // Should be replaced by the parameter payload.
    std::vector<uint8_t> payloadPacket = {0xFF, 0xFF, 0xFF, 0xFF}; // Payload Size is 4 byte long in the simulation, 40 Bytes max in real life
    std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00};  // Hash Function is 4 byte long in the simulation AND in real life

    // Append all fields

    appendVector(dataPacket, std::vector<uint8_t>{static_cast<uint8_t>(common::PacketType::RRC_Uplink_Data)});
    appendVector(dataPacket, senderGlobalId);
    appendVector(dataPacket, receiverGlobalId);
    appendVector(dataPacket, localIDPacket);
    appendVector(dataPacket, payloadPacket);
    appendVector(dataPacket, hashFunction);

    // === METRICS: Track packet transmission ===
    if (node.metricsAggregator && node.metrics) {
        // Assign global packet ID
        GlobalPacketID globalId = node.metricsAggregator->assignGlobalPacketId();

        // Register mapping: (nodeId, localId) -> globalId for ACK correlation
        node.metricsAggregator->registerPacketKey(
            node.getId(),
            node.RRC_UPLINK_localIDPacketCounter,
            globalId
        );

        // Record packet transmission in node metrics (unified PDR tracking)
        node.metrics->recordPacketTransmission(
            globalId,
            node.RRC_UPLINK_infoFromBeaconPhase->getNextNodeIdInPath(),
            node.getCurrentTime()
        );

        // Record in global aggregator for latency tracking
        // Check if this is a forwarded packet or originated
        if (!node.RRC_UPLINK_forwardingQueue.empty()) {
            // This is a forwarded packet - pop the received globalId and link them
            GlobalPacketID receivedGlobalId = node.RRC_UPLINK_forwardingQueue.front();
            node.RRC_UPLINK_forwardingQueue.pop();

            node.metricsAggregator->recordPacketForwarded(
                node.getId(),
                receivedGlobalId,
                globalId,
                node.getCurrentTime()
            );
        } else {
            // This is an originated packet
            node.metricsAggregator->recordPacketOriginated(
                node.getId(),
                globalId,
                node.getCurrentTime()
            );
        }
    }
    //Todo: add an else statement and a warning
    
    // virtualization of the Transmit() from Physical Layer
    node.addMessageToTransmit(dataPacket, common::rrc_uplink::timeOnAirDataPacket_ms);
}

void C2_RRC_UplinkHandler::buildAndTransmitAckPacket(C2_Node &node)
{
    std::vector<uint8_t> ackPacket;

    // preallocate the space for optimization
    ackPacket.reserve(common::field_sizes::type +
                      common::field_sizes::global_id + // Sender
                      common::field_sizes::global_id + // receiver
                      common::rrc_uplink::localIDPacketBytesSize +
                      common::field_sizes::hash);

    // prepare the fields:
    std::vector<uint8_t> senderGlobalId = decimalToBytes(node.nodeId, common::field_sizes::global_id); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life

    auto ackInformationIds = node.RRC_UPLINK_ackInformation->getAndResetAckInformation();
    std::vector<uint8_t> receiverGlobalId = decimalToBytes(ackInformationIds.first, common::field_sizes::global_id); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> localIDPacket = decimalToBytes(ackInformationIds.second, common::localIDPacketBytesSize);   // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00};                                                    // Hash Function is 4 byte long in the simulation AND in real life

    // Append all fields
    appendVector(ackPacket, std::vector<uint8_t>{static_cast<uint8_t>(common::PacketType::RRC_Uplink_ACK)});
    appendVector(ackPacket, senderGlobalId);
    appendVector(ackPacket, receiverGlobalId);
    appendVector(ackPacket, localIDPacket);
    appendVector(ackPacket, hashFunction);

    // virtualization of the Transmit() from Physical Layer
    // we add the message to the transmission queue, so it will be sent at the next transmission slot
    node.addMessageToTransmit(ackPacket, common::rrc_uplink::timeOnAirAckPacket_ms);

    node.adressedPacketTransmissionDisplay(ackInformationIds.first, true);
}
