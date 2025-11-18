#include "C2_Node.hpp"
#include "../../Miscellaneous/Utilities/Utilities.hpp"
#include "RRC_Uplink/C2_RRC_UplinkHandler.hpp"
#include "RRC_Uplink/C2_RRC_UplinkSlotManager.hpp"


//TODO: clean the comments once you have made the two others RRC Modes (Downlink+beacons), you might want to reuse parts of the code already done
using namespace common;
using namespace packet_tool;

C2_Node::C2_Node(int id, Logger &logger, std::pair<int, int> coordinates)
    : Node(id, logger, coordinates)
{
    nodeClass = NodeClass::C2;
    initializeTransitionMap();
    setCurrentState(NodeState::Sleeping);
};

C2_Node::C2_Node(int id, Logger &logger, std::pair<int, int> coordinates, uint16_t nextNodeIdInPath, uint8_t hopCount)
    : Node(id, logger, coordinates)
{
    nodeClass = NodeClass::C2;

    RRC_UPLINK_infoFromBeaconPhase = std::make_unique<InformationFromBeaconPhase>(nextNodeIdInPath, hopCount);
    RRC_UPLINK_slotManager = std::make_unique<C2_RRC_UplinkSlotManager>();
    RRC_UPLINK_ackInformation = std::make_unique<AckInformation>();
    RRC_UPLINK_retransmissionCounterHelper = std::make_unique<RetransmissionCounterHelper>();

    initializeTransitionMap();
    setCurrentState(NodeState::Sleeping);

    // decide which slots among the  speci will actually be possible DATA and modulo slots actually used to transmit information
    RRC_UPLINK_slotManager->initializeRandomSlots(common::maxNodeSlots, common::totalNumberOfSlotsPerModuloNode);

    // Build the message string showing the selected slots
    std::string slotsLogMsg = "Transmission slots: [";
    const auto &slots = RRC_UPLINK_slotManager->getSlots();

    for (size_t i = 0; i < slots.size(); ++i)
    {
        slotsLogMsg += std::to_string(slots[i]);
        if (i != slots.size() - 1)
        {
            slotsLogMsg += ", ";
        }
    }
    slotsLogMsg += "]";

    // The node will use the allowed category of slots to transmit
    RRC_UPLINK_fixedSlotCategory = RRC_UPLINK_infoFromBeaconPhase->getHopCount() % 3;

    // Initialize packet queue with originated packets
    for (uint8_t i = 0; i < RRC_UPLINK_initialnbPayload; i++) {
        packet_buffer::PacketBuffer packet;
        packet.origin = packet_buffer::PacketBuffer::PacketOrigin::Originated;
        packet.globalPacketId = std::nullopt;  // Assigned at transmission time
        packet.payload = {0xFF, 0xFF, 0xFF, 0xFF};
        packet.arrivalTime = 0;  // Initial packets created at time 0
        RRC_UPLINK_packetQueue.push(std::move(packet));
    }
};

C2_Node::~C2_Node() = default;




std::string C2_Node::initMessage() const
{

    std::string msg = Node::initMessage();
    std::string finalMsg = msg + "Class: " + std::to_string(getClassId()) + " started to run";

    sf::Packet positionPacketReceiver;
    positionPacket initialPacket;
    if (RRC_UPLINK_infoFromBeaconPhase)
        initialPacket = positionPacket(nodeId, 2, coordinates, batteryLevel, RRC_UPLINK_infoFromBeaconPhase->getHopCount());
    else
        initialPacket = positionPacket(nodeId, 2, coordinates, batteryLevel, -1);
    positionPacketReceiver << initialPacket;
    logger.sendTcpPacket(positionPacketReceiver);

    return finalMsg;
}

ModeHandler<C2_Node>* C2_Node::getCurrentHandler() {
    if (!blueprint) {
        throw std::runtime_error("Node " + std::to_string(nodeId) + " has no blueprint set");
    }
    
    int64_t currentTime = getCurrentTime();
    TdmaMode currentMode = blueprint->getCurrentMode(currentTime);
    
    auto it = modeHandlers.find(currentMode);
    if (it == modeHandlers.end()) {
        throw std::runtime_error("Node " + std::to_string(nodeId) + 
                                 " has no handler for mode: " + std::to_string(static_cast<int>(currentMode)));
    }
    
    return it->second.get();
}

// #if COMMUNICATION_PERIOD == RRC_BEACON

// bool C2_Node::canNodeReceiveMessage()
// {
//     // State Condition: node must be listening to receive a message
//     if (currentState != NodeState::Listening && currentState != NodeState::Communicating)
//     {
//         return false;
//     }
//     else if (currentState == NodeState::Communicating)
//     {
//         return !isTransmittingWhileCommunicating;
//     }
//     return true;
// }

// bool C2_Node::receiveMessage(const std::vector<uint8_t> message)
// {
//     if (!canNodeReceiveMessage())
//     {
            //use the new enum ReceptionState and helper in Node.hpp
//         sf::Packet receptionStatePacketReceiver;
//         uint16_t senderId = extractBytesFromField(message, senderGlobalId, common::rrc_beacon_fields);
//         receiveMessagePacket receptionState(senderId, nodeId, "notListening");
//         receptionStatePacketReceiver << receptionState;
//         logger.sendTcpPacket(receptionStatePacketReceiver);

//         return false;
//     }

//     uint16_t senderId = extractBytesFromField(message, senderGlobalId, common::rrc_beacon_fields);
//     sf::Packet receptionStatePacketReceiver;

//     if (!Node::receiveMessage(message))
//     {
//         // an interference happened, we don't treat the message

//         receiveMessagePacket receptionState(senderId, nodeId, "interference");
//         receptionStatePacketReceiver << receptionState;
//         logger.sendTcpPacket(receptionStatePacketReceiver);
//         return false;
//     }

//     receiveMessagePacket receptionState(senderId, nodeId, "received");
//     receptionStatePacketReceiver << receptionState;
//     logger.sendTcpPacket(receptionStatePacketReceiver);

//     // TODO: implement this type check in every Mode !!
//     if (getPacketType(message, common::rrc_beacon_fields) != PacketType::RRC_Beacon)
//     {
//         // not a beacon, we don't care

//         // drop Message
//         dropAnimationPacket dropPacket(nodeId);
//         sf::Packet dropPacketReceiver;
//         dropPacketReceiver << dropPacket;
//         logger.sendTcpPacket(dropPacketReceiver);
//         // receiveBuffer.pop();
//         return false;
//     }

//     uint8_t packetHopCount = extractBytesFromField(message, common::field_names::hopCount, common::rrc_beacon_fields);
//     uint32_t packetTimeStamp = extractBytesFromField(message, common::field_names::timeStamp, common::rrc_beacon_fields);
//     uint16_t packetGlobalIDPacket = extractBytesFromField(message, common::field_names::globalIDPacket, common::rrc_beacon_fields);
//     uint8_t packetPathCost = extractBytesFromField(message, common::field_names::costFunction, common::rrc_beacon_fields);
//     uint16_t packetNextNodeIdInPath = extractBytesFromField(message, common::field_names::senderGlobalId, common::rrc_beacon_fields);

//     if (!hopCount.has_value())
//     {
//         // this is the first beacon received
//         shouldSendBeacon = true; // next tranmission slots, will create the new sending beacon scheduler
//         hopCount = packetHopCount + 1;
//         lastTimeStampReceived = packetTimeStamp;
//         globalIDPacketList.push_back(packetGlobalIDPacket);
//         basePathCost = packetPathCost;
//         // The increment should increase as the battery level decreases. For now, simple linear  increment function.
//         uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
//                                                      (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
//         pathCost = increment;

//         if (nextNodeIdInPath.has_value())
//         {
//             // we supress the old routing in the visualiser.....
//             sf::Packet routingPacketReceiver;
//             routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), false);
//             routingPacketReceiver << routingPacket;
//             logger.sendTcpPacket(routingPacketReceiver);
//         }

//         nextNodeIdInPath = packetNextNodeIdInPath;
//         sf::Packet routingPacketReceiver;
//         routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), true);
//         routingPacketReceiver << routingPacket;
//         logger.sendTcpPacket(routingPacketReceiver);

//         return true;
//     }
//     else
//     {
//         if (nextNodeIdInPath == packetNextNodeIdInPath)
//         {
//             // we received a Beacon from the optimized path, but we need to check if the associated cost changed
//             if (basePathCost < packetPathCost)
//             {
//                 // the cost has changed
//                 uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
//                                                              (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
//                 pathCost = increment;
//                 basePathCost = packetPathCost;
//                 hopCount = packetHopCount + 1; // we add one to the hop count / it can happen that the next Optimal Node in the path found a new optimal path itself, thus changing the hop count
//             }
//         }
//         else
//         {
//             // check if the registerd path is still the least costly, otherwise update the path
//             if (basePathCost > packetPathCost)
//             {
//                 // the optimized path has changed - the path is independent from the fact we resend beacon
//                 uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
//                                                              (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
//                 pathCost = increment;
//                 basePathCost = packetPathCost;
//                 hopCount = packetHopCount + 1;

//                 // we supress the old routing in the visualiser....
//                 sf::Packet routingPacketReceiver;
//                 routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), false);
//                 routingPacketReceiver << routingPacket;
//                 logger.sendTcpPacket(routingPacketReceiver);

//                 nextNodeIdInPath = packetNextNodeIdInPath;

//                 logEvent("Forget old route, route through " + std::to_string(nextNodeIdInPath.value()));

//                 // and add the new one
//                 sf::Packet routingPacketReceiver2;
//                 routingDecisionPacket routingPacket2(nodeId, nextNodeIdInPath.value(), true);
//                 routingPacketReceiver2 << routingPacket2;
//                 logger.sendTcpPacket(routingPacketReceiver2);
//             }
//         }

//         // TODO: put the +4 in common
//         // updating the clock and changing the route are two independant process!
//         if (hopCount.value() + 4 > packetHopCount)
//         { // this +4 is my gut feeling, should be studied
//             // the timestamp received can be included in the synchronization clock mechanism as it has a similar relative accuracy

//             lastTimeStampReceived = packetTimeStamp; // there should be a function to actualize the internal clock here

//             if (std::find(globalIDPacketList.begin(), globalIDPacketList.end(), packetGlobalIDPacket) != globalIDPacketList.end())
//             {
//                 logEvent("Dropping duplicate beacon from Node " + std::to_string(nodeId));
//                 // drop Message
//                 dropAnimationPacket dropPacket(nodeId);
//                 sf::Packet dropPacketReceiver;
//                 dropPacketReceiver << dropPacket;
//                 logger.sendTcpPacket(dropPacketReceiver);
//             }
//             else
//             {
//                 // it's a new beacon, we reenter boradcast mode regardless of the beacons left to send
//                 globalIDPacketList.push_back(packetGlobalIDPacket);
//                 shouldSendBeacon = true; // at next transmission slots, will create the new sending beacon scheduler
//                 beaconSlots.clear();
//             }
//         }

//         return true; // it's for the compiler to not throw a warning, we never capture this variable
//     }
// }

// //----------------------------STATE TRANSITIONS--------------------------------

// bool C2_Node::canSleepFromCommunicating()
// { // Node Can alwasy sleep
//     sf::Packet statePacketReceiver;
//     stateNodePacket statePacket(nodeId, "Sleep");
//     statePacketReceiver << statePacket;
//     logger.sendTcpPacket(statePacketReceiver);
//     // Todo: Protect with mutexes.

//     currentState = NodeState::Sleeping;
//     // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
//     // logger.logMessage(transitionLog);
//     return true;
// }

// bool C2_Node::canCommunicateFromSleeping()
// {
//     // TODO: put these States "Communicate", "Sleep" in Common.hpp to have a common reference !!
//     sf::Packet statePacketReceiver;
//     stateNodePacket statePacket(nodeId, "Communicate");
//     statePacketReceiver << statePacket;
//     logger.sendTcpPacket(statePacketReceiver);

//     isTransmittingWhileCommunicating = false;
//     // Todo: Protect with mutexes.

//     currentState = NodeState::Communicating;
//     if (shouldSendBeacon && beaconSlots.size() == 0)
//     {
//         // a "new" beacon has just been received, we plan the random slots
//         shouldSendBeacon = false;
//         beaconSlots = simulation_util::selectRandomSlots(simulation_util::computeRandomNbBeaconPackets(common::rrc_beacon::minimumNbBeaconPackets, common::rrc_beacon::maximumNbBeaconPackets), common::rrc_beacon::nbSlotsPossibleForOneBeacon);
//         std::ostringstream oss;
//         for (size_t i = 0; i < beaconSlots.size(); ++i)
//         {
//             oss << beaconSlots[i];
//             if (i < beaconSlots.size() - 1)
//             {
//                 oss << ", "; // Add a separator between elements
//             }
//         }

//     }
//     if (beaconSlots.size() > 0)
//     {
//         // we have beacons to send
//         if (beaconSlots[0] == 0)
//         {
//             isTransmittingWhileCommunicating = true;

//             //GUARD TIME WITH NEW SYSTEM??
//             // std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

//             // create the beacon packet
//             std::vector<uint8_t> beaconPacket;
//             // preallocate the space for optimization
//             beaconPacket.reserve(common::field_sizes::type + common::rrc_beacon::timeStampBytesSize + common::costFunctionBytesSize +
//                                  common::rrc_beacon::hopCountBytesSize + common::field_sizes::global_id_packet +
//                                  common::field_sizes::global_id + common::field_sizes::hash);

//             // prepare the fields
//             //TODO: Timestamp?
//             std::vector<uint8_t> newTimeStamp = {0x0,0x0,0x0,0x0};
//             std::vector<uint8_t> newCostFunction = {pathCost.value()};
//             std::vector<uint8_t> newHopCount = decimalToBytes(hopCount.value(), common::rrc_beacon::hopCountBytesSize);
//             std::vector<uint8_t> newGlobalIDPacket = decimalToBytes(globalIDPacketList.back(), common::field_sizes::global_id_packet);
//             std::vector<uint8_t> newSenderGlobalId = decimalToBytes(nodeId, common::field_sizes::global_id);
//             std::vector<uint8_t> newHashFunction = {0x00, 0x00, 0x00, 0x00};

//             // Append all fields
//             appendVector(beaconPacket, std::vector<uint8_t>{static_cast<uint8_t>(common::PacketType::RRC_Beacon)});
//             appendVector(beaconPacket, newTimeStamp);
//             appendVector(beaconPacket, newCostFunction);
//             appendVector(beaconPacket, newHopCount);
//             appendVector(beaconPacket, newGlobalIDPacket);
//             appendVector(beaconPacket, newSenderGlobalId);
//             appendVector(beaconPacket, newHashFunction);

//             sf::Packet broadcastPacketReceiver;
//             broadcastMessagePacket broadcastPacket(nodeId);
//             broadcastPacketReceiver << broadcastPacket;
//             logger.sendTcpPacket(broadcastPacketReceiver);

//             addMessageToTransmit(beaconPacket, common::rrc_beacon::timeOnAirBeacon_ms);
//             beaconSlots.erase(beaconSlots.begin());
//             logEvent("Tx Beacon..");}
//         if (!beaconSlots.empty())
//         {
//             // decrease every elements of the slots by one
//             for (int i = 0; i < beaconSlots.size(); i++)
//             {
//                 beaconSlots[i]--;
//             }
//         }
//     }
//     return true;
// }

// // Unauthorized transition in this mode.
// bool C2_Node::canCommunicateFromTransmitting() { return false; }
// bool C2_Node::canCommunicateFromListening() { return false; }
// bool C2_Node::canCommunicateFromCommunicating() { return true; }
// bool C2_Node::canTransmitFromListening() { return true; }
// bool C2_Node::canTransmitFromSleeping() { return true; }
// bool C2_Node::canTransmitFromTransmitting() { return true; }
// bool C2_Node::canTransmitFromCommunicating() { return false; }
// bool C2_Node::canListenFromTransmitting() { return true; }
// bool C2_Node::canListenFromSleeping() { return false; }
// bool C2_Node::canListenFromListening() { return false; }
// bool C2_Node::canListenFromCommunicating() { return false; }
// bool C2_Node::canSleepFromTransmitting() { return false; }
// bool C2_Node::canSleepFromListening() { return false; }
// bool C2_Node::canSleepFromSleeping() { return false; }

// #elif COMMUNICATION_PERIOD == RRC_DOWNLINK

// bool C2_Node::canNodeReceiveMessage()
// {
//     // Todo: Protect with mutexes.

//     // State Condition: node must be listening to receive a message
//     if (currentState != NodeState::Listening && currentState != NodeState::Communicating)
//     {
//         return false;
//     }
//     else if (currentState == NodeState::Communicating)
//     {
//         return !isTransmittingWhileCommunicating;
//     }
//     return true;
// }

// bool C2_Node::receiveMessage(const std::vector<uint8_t> message)
// {
//     sf::Packet receptionStatePacketReceiver;
//     uint16_t senderId = extractBytesFromField(message, common::field_names::senderGlobalId, common::rrc_downlink_fields);

//     if (!canNodeReceiveMessage())
//     {
//         receiveMessagePacket receptionState(senderId, nodeId, "notListening");
//         receptionStatePacketReceiver << receptionState;
//         logger.sendTcpPacket(receptionStatePacketReceiver);

//         return false;
//     }

//     if (!Node::receiveMessage(message))
//     {
//         // an interference happened, we don't treat the message
//         receiveMessagePacket receptionState(senderId, nodeId, "interference");
//         receptionStatePacketReceiver << receptionState;
//         logger.sendTcpPacket(receptionStatePacketReceiver);

//         return false;
//     }

//     if (getPacketType(message, common::rrc_downlink_fields) != PacketType::RRC_Downlink)
//     {
//         // not a downlink packet, we don't care
//         logEvent("drop pck");
//         // receiveBuffer.pop();
//         return false;
//     }

//     receiveMessagePacket receptionState(senderId, nodeId, "received");
//     receptionStatePacketReceiver << receptionState;
//     logger.sendTcpPacket(receptionStatePacketReceiver);

//     uint8_t packetSenderId = extractBytesFromField(message, field_names::senderGlobalId, common::rrc_downlink_fields);
//     uint32_t packetReceiverId = extractBytesFromField(message, field_names::receiverGlobalId, common::rrc_downlink_fields);
//     uint16_t packetGlobalIDPacket = extractBytesFromField(message, field_names::globalIDPacket, common::rrc_downlink_fields);
//     uint8_t packetPayload = extractBytesFromField(message, field_names::payload, common::rrc_downlink_fields);
//     uint8_t packetHashFunction = extractBytesFromField(message, field_names::hashFunction, common::rrc_downlink_fields);

//     if (packetReceiverId == nodeId)
//     { // add C1 Childs condition when you implement more complex topology
//         // THe packet is for us!
//         logEvent("Pck rcv");
//         return true;
//     }
//     if (globalIDPacketList.empty())
//     {
//         // I was lazy so I copy past code from beacon mode, it's relatively the same, hence the disturbing name variables TODO change that
//         // this is the first beacon received
//         shouldSendBeacon = true; // next tranmission slots, will create the new sending beacon scheduler
//         globalIDPacketList.push_back(packetGlobalIDPacket);
//         packetFinalReceiverId = packetReceiverId;
//         return true;
//     }
//     else
//     {

//         if (std::find(globalIDPacketList.begin(), globalIDPacketList.end(), packetGlobalIDPacket) != globalIDPacketList.end())
//         {
//             logEvent("drop pck");
//             dropAnimationPacket dropPacket(nodeId);
//             sf::Packet dropPacketReceiver;
//             dropPacketReceiver << dropPacket;
//             logger.sendTcpPacket(dropPacketReceiver);
//         }
//         else
//         {
//             // this is another downlink message. The node stops sending former messages and restars with new
//             globalIDPacketList.push_back(packetGlobalIDPacket);
//             shouldSendBeacon = true; // at next transmission slots, will create the new sending beacon scheduler
//             packetFinalReceiverId = packetReceiverId;

//             // maybe don't clear if you want to keep the former messages to be sent yet
//             beaconSlots.clear();
//         }
//     }

//     return true; // it's for the compiler to not throw a warning, we never capture this variable
// }

// //----------------------------STATE TRANSITIONS--------------------------------

// bool C2_Node::canCommunicateFromSleeping()
// {

//     sf::Packet statePacketReceiver;
//     stateNodePacket statePacket(nodeId, "Communicate");
//     statePacketReceiver << statePacket;
//     logger.sendTcpPacket(statePacketReceiver);

//     isTransmittingWhileCommunicating = false;
//     // Todo: Protect with mutexes.

//     currentState = NodeState::Communicating;
//     if (shouldSendBeacon && beaconSlots.size() == 0)
//     {
//         // a "new" beacon has just been received, we plan the random slots
//         shouldSendBeacon = false;
//         beaconSlots = simulation_util::selectRandomSlots(simulation_util::computeRandomNbBeaconPackets(common::rrc_downlink::minimumNbBeaconPackets, common::rrc_downlink::maximumNbBeaconPackets), common::rrc_downlink::nbSlotsPossibleForOneBeacon);
//         std::ostringstream oss;
//         for (size_t i = 0; i < beaconSlots.size(); ++i)
//         {
//             oss << beaconSlots[i];
//             if (i < beaconSlots.size() - 1)
//             {
//                 oss << ", "; // Add a separator between elements
//             }
//         }
//         // Log beaconSlotsLog("Node "+std::to_string(nodeId)+" will send beacons at slots: "+oss.str(), true);
//         // logger.logMessage(beaconSlotsLog);
//     }
//     if (beaconSlots.size() > 0)
//     {
//         // we have beacons to send
//         if (beaconSlots[0] == 0)
//         {
//             isTransmittingWhileCommunicating = true;

//             //Todo: guard time?
//             // std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

//             //-------------------------------------define Beacon Packet----------------------------
//             //----------least signficant byte first (little endian) !------------
//             std::vector<uint8_t> globalIDPacket = decimalToBytes(globalIDPacketList.back(), common::field_sizes::global_id_packet); // Global ID is 2 byte long in the simulation, 10 bits in real life
//             std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId, common::field_sizes::global_id);                                                  // Sender Global ID is 2 byte long in the simulation, 10 bits in real life

//             std::vector<uint8_t> receiverGlobalId = decimalToBytes(packetFinalReceiverId.value(), common::field_sizes::global_id); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
//             std::vector<uint8_t> payload = {0xFF, 0xFF, 0xFF, 0xFF};                                  // Payload Size is 4 byte long in the simulation, 40 Bytes max in real life

//             // dummy hash: we don't implement the hash function in this simulation
//             std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00}; // Hash Function is 4 byte long in the simulation AND in real life

//             // Concatenate fields into one vector
//             std::vector<uint8_t> beaconPacket;

//             // preallocate the space for optimization
//             // TODO: should use the size in the common file, not the variable, source of error
//             beaconPacket.reserve(common::field_sizes::type +
//                                  common::field_sizes::global_id + //Sender
//                                  common::field_sizes::global_id + //receiver
//                                  common::field_sizes::global_id_packet +
//                                  payload.size() +
//                                  common::field_sizes::hash);

//             // Append all fields
//             appendVector(beaconPacket, std::vector<uint8_t>{static_cast<uint8_t>(common::PacketType::RRC_Downlink)});
//             appendVector(beaconPacket, senderGlobalId);
//             appendVector(beaconPacket, receiverGlobalId);
//             appendVector(beaconPacket, globalIDPacket);
//             appendVector(beaconPacket, payload);
//             appendVector(beaconPacket, hashFunction);

//             sf::Packet broadcastPacketReceiver;
//             broadcastMessagePacket broadcastPacket(nodeId);
//             broadcastPacketReceiver << broadcastPacket;
//             logger.sendTcpPacket(broadcastPacketReceiver);

//             addMessageToTransmit(beaconPacket, common::rrc_downlink::timeOnAirFlood_ms);
//             beaconSlots.erase(beaconSlots.begin());
//         }
//         if (!beaconSlots.empty())
//         {
//             // decrease every elements of the slots by one
//             for (int i = 0; i < beaconSlots.size(); i++)
//             {
//                 beaconSlots[i]--;
//             }
//         }
//     }
//     return true;
// }

// bool C2_Node::canSleepFromCommunicating()
// { // Node Can alwasy sleep

//     sf::Packet statePacketReceiver;
//     stateNodePacket statePacket(nodeId, "Sleep");
//     statePacketReceiver << statePacket;
//     logger.sendTcpPacket(statePacketReceiver);

//     // Todo: Protect with mutexes.
//     currentState = NodeState::Sleeping;
//     // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
//     // logger.logMessage(transitionLog);
//     return true;
// }
// // Unauthorized transition in this mode.
// bool C2_Node::canCommunicateFromTransmitting() { return false; }
// bool C2_Node::canCommunicateFromListening() { return false; }
// bool C2_Node::canCommunicateFromCommunicating() { return true; }
// bool C2_Node::canTransmitFromListening() { return true; }
// bool C2_Node::canTransmitFromSleeping() { return true; }
// bool C2_Node::canTransmitFromTransmitting() { return true; }
// bool C2_Node::canTransmitFromCommunicating() { return false; }
// bool C2_Node::canListenFromTransmitting() { return true; }
// bool C2_Node::canListenFromSleeping() { return false; }
// bool C2_Node::canListenFromListening() { return false; }
// bool C2_Node::canListenFromCommunicating() { return false; }
// bool C2_Node::canSleepFromTransmitting() { return false; }
// bool C2_Node::canSleepFromListening() { return false; }
// bool C2_Node::canSleepFromSleeping() { return false; }

// #elif COMMUNICATION_PERIOD == RRC_UPLINK

// TODO: move it in C2_RC_UplinkHandler
void C2_Node::RRC_UPLINK_displayRouting()
{
    if (RRC_UPLINK_infoFromBeaconPhase)
    {
        sf::Packet routingPacketReceiver;
        routingDecisionPacket routingPacket(nodeId, RRC_UPLINK_infoFromBeaconPhase->getNextNodeIdInPath(), true);
        routingPacketReceiver << routingPacket;
        logger.sendTcpPacket(routingPacketReceiver);
    }
}

// Delegate all state transitions to current handler
bool C2_Node::canTransmitFromListening() {
    return getCurrentHandler()->canTransmitFromListening(*this);
}

bool C2_Node::canTransmitFromSleeping() {
    return getCurrentHandler()->canTransmitFromSleeping(*this);
}

bool C2_Node::canTransmitFromTransmitting() {
    return getCurrentHandler()->canTransmitFromTransmitting(*this);
}

bool C2_Node::canTransmitFromCommunicating() {
    return getCurrentHandler()->canTransmitFromCommunicating(*this);
}

bool C2_Node::canListenFromTransmitting() {
    return getCurrentHandler()->canListenFromTransmitting(*this);
}

bool C2_Node::canListenFromSleeping() {
    return getCurrentHandler()->canListenFromSleeping(*this);
}

bool C2_Node::canListenFromListening() {
    return getCurrentHandler()->canListenFromListening(*this);
}

bool C2_Node::canListenFromCommunicating() {
    return getCurrentHandler()->canListenFromCommunicating(*this);
}

bool C2_Node::canSleepFromTransmitting() {
    return getCurrentHandler()->canSleepFromTransmitting(*this);
}

bool C2_Node::canSleepFromListening() {
    return getCurrentHandler()->canSleepFromListening(*this);
}

bool C2_Node::canSleepFromSleeping() {
    return getCurrentHandler()->canSleepFromSleeping(*this);
}

bool C2_Node::canSleepFromCommunicating() {
    return getCurrentHandler()->canSleepFromCommunicating(*this);
}

bool C2_Node::canCommunicateFromTransmitting() {
    return getCurrentHandler()->canCommunicateFromTransmitting(*this);
}

bool C2_Node::canCommunicateFromListening() {
    return getCurrentHandler()->canCommunicateFromListening(*this);
}

bool C2_Node::canCommunicateFromSleeping() {
    return getCurrentHandler()->canCommunicateFromSleeping(*this);
}

bool C2_Node::canCommunicateFromCommunicating() {
    return getCurrentHandler()->canCommunicateFromCommunicating(*this);
}

void C2_Node::handleCommunication() {
    getCurrentHandler()->handleCommunication(*this);
}

bool C2_Node::receiveMessage(const std::vector<uint8_t> message) {
    return getCurrentHandler()->receiveMessage(*this, message);
}

bool C2_Node::canNodeReceiveMessage() {
    return getCurrentHandler()->canNodeReceiveMessage(*this);
}