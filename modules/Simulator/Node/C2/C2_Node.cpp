#include "C2_Node.hpp"

std::string C2_Node::initMessage() const
{

    std::string msg = Node::initMessage();
    std::string finalMsg = msg + "Class: " + std::to_string(getClassId()) + " started to run";

    sf::Packet positionPacketReceiver;
    positionPacket positionPacket(nodeId, 2, coordinates, batteryLevel, infoFromBeaconPhase.getHopCount());
    positionPacketReceiver << positionPacket;
    logger.sendTcpPacket(positionPacketReceiver);

    // if (nextNodeIdInPath.has_value() && nextNodeIdInPath.value() != -1)
    // {
    //     sf::Packet routePacket;
    //     routingDecisionPacket routeDecisionPacket(nodeId, nextNodeIdInPath.value(), true);
    //     routePacket << routeDecisionPacket;
    // }
#if TOPOLOGY == MESH_SELF_HEALING
    if (nextNodeIdInPath.has_value())
    {
        sf::Packet routePacket;
        routingDecisionPacket routeDecisionPacket(nodeId, nextNodeIdInPath.value(), true);
        routePacket << routeDecisionPacket;
        logger.sendTcpPacket(routePacket);
    }
#endif
    return finalMsg;
}


#if COMMUNICATION_PERIOD == RRC_BEACON

bool C2_Node::canNodeReceiveMessage()
{
    // State Condition: node must be listening to receive a message
    if (currentState != NodeState::Listening && currentState != NodeState::Communicating)
    {
        return false;
    }
    else if (currentState == NodeState::Communicating)
    {
        return !isTransmittingWhileCommunicating;
    }
    return true;
}

bool C2_Node::receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir)
{
    if (!canNodeReceiveMessage())
    {
        Log notlisteninglog("Node " + std::to_string(nodeId) + " not listening, dropped msg" /*+detailedBytesToString( message)*/, true);
        logger.logMessage(notlisteninglog);

        sf::Packet receptionStatePacketReceiver;
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
        receiveMessagePacket receptionState(senderId, nodeId, "notListening");
        receptionStatePacketReceiver << receptionState;
        logger.sendTcpPacket(receptionStatePacketReceiver);

        return false;
    }

    if (!Node::receiveMessage(message, timeOnAir))
    {
        // an interference happened, we don't treat the message

        sf::Packet receptionStatePacketReceiver;
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
        receiveMessagePacket receptionState(senderId, nodeId, "interference");
        receptionStatePacketReceiver << receptionState;
        logger.sendTcpPacket(receptionStatePacketReceiver);
        return false;
    }

    sf::Packet receptionStatePacketReceiver;
    uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
    receiveMessagePacket receptionState(senderId, nodeId, "received");
    receptionStatePacketReceiver << receptionState;
    logger.sendTcpPacket(receptionStatePacketReceiver);

    uint8_t type = extractBytesFromField(message, "type", common::fieldMap);
    if (type != common::type[0])
    {
        // not a beacon, we don't care
        // TODO: implement this type check in every Mode !!
        Log wrongTypeLog("Node " + std::to_string(nodeId) + " received Incorrecty packet type, dropping", true);
        logger.logMessage(wrongTypeLog);

        // drop Message
        dropAnimationPacket dropPacket(nodeId);
        sf::Packet dropPacketReceiver;
        dropPacketReceiver << dropPacket;
        logger.sendTcpPacket(dropPacketReceiver);
        // receiveBuffer.pop();
        return false;
    }

    uint8_t packetHopCount = extractBytesFromField(message, "hopCount", common::fieldMap);
    uint32_t packetTimeStamp = extractBytesFromField(message, "timeStamp", common::fieldMap);
    uint16_t packetGlobalIDPacket = extractBytesFromField(message, "globalIDPacket", common::fieldMap);
    uint8_t packetPathCost = extractBytesFromField(message, "costFunction", common::fieldMap);
    uint16_t packetNextNodeIdInPath = extractBytesFromField(message, "senderGlobalId", common::fieldMap);

    std::lock_guard<std::mutex> lock(receiveMutex);
    if (!hopCount.has_value())
    {
        // this is the first beacon received
        shouldSendBeacon = true; // next tranmission slots, will create the new sending beacon scheduler
        hopCount = packetHopCount + 1;
        lastTimeStampReceived = packetTimeStamp;
        globalIDPacketList.push_back(packetGlobalIDPacket);
        basePathCost = packetPathCost;
        // The increment should increase as the battery level decreases. For now, simple linear  increment function.
        uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
                                                     (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
        pathCost = increment;

        if (nextNodeIdInPath.has_value())
        {
            // we supress the old routing in the visualiser.....
            sf::Packet routingPacketReceiver;
            routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), false);
            routingPacketReceiver << routingPacket;
            logger.sendTcpPacket(routingPacketReceiver);
        }

        nextNodeIdInPath = packetNextNodeIdInPath;
        sf::Packet routingPacketReceiver;
        routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), true);
        routingPacketReceiver << routingPacket;
        logger.sendTcpPacket(routingPacketReceiver);
        Log rootingLog("Node " + std::to_string(nodeId) + " rooting with Node:" + std::to_string(nextNodeIdInPath.value()), true);
        logger.logMessage(rootingLog);

        return true;
    }
    else
    {
        if (nextNodeIdInPath == packetNextNodeIdInPath)
        {
            // we received a Beacon from the optimized path, but we need to check if the associated cost changed
            if (basePathCost < packetPathCost)
            {
                // the cost has changed
                uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
                                                             (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
                pathCost = increment;
                basePathCost = packetPathCost;
                hopCount = packetHopCount + 1; // we add one to the hop count / it can happen that the next Optimal Node in the path found a new optimal path itself, thus changing the hop count
            }
        }
        else
        {
            // check if the registerd path is still the least costly, otherwise update the path
            if (basePathCost > packetPathCost)
            {
                // the optimized path has changed - the path is independent from the fact we resend beacon
                uint8_t increment = packetPathCost + 4 * static_cast<uint8_t>(
                                                             (10.0 - 1.0) / 100.0 * (100.0 - batteryLevel));
                pathCost = increment;
                basePathCost = packetPathCost;
                hopCount = packetHopCount + 1;

                // we supress the old routing in the visualiser....
                sf::Packet routingPacketReceiver;
                routingDecisionPacket routingPacket(nodeId, nextNodeIdInPath.value(), false);
                routingPacketReceiver << routingPacket;
                logger.sendTcpPacket(routingPacketReceiver);

                Log oldRootingLog("Node " + std::to_string(nodeId) + " FORGETTING rooting with Node:" + std::to_string(nextNodeIdInPath.value()), true);
                logger.logMessage(oldRootingLog);

                nextNodeIdInPath = packetNextNodeIdInPath;

                Log rootingLog("Node " + std::to_string(nodeId) + " rooting with Node:" + std::to_string(nextNodeIdInPath.value()), true);
                logger.logMessage(rootingLog);

                // and add the new one
                sf::Packet routingPacketReceiver2;
                routingDecisionPacket routingPacket2(nodeId, nextNodeIdInPath.value(), true);
                routingPacketReceiver2 << routingPacket2;
                logger.sendTcpPacket(routingPacketReceiver2);
            }
        }

        // TODO: put the +4 in common
        // updating the clock and changing the route are two independant process!
        if (hopCount.value() + 4 > packetHopCount)
        { // this +4 is my gut feeling, should be studied
            // the timestamp received can be included in the synchronization clock mechanism as it has a similar relative accuracy

            lastTimeStampReceived = packetTimeStamp; // there should be a function to actualize the internal clock here

            if (std::find(globalIDPacketList.begin(), globalIDPacketList.end(), packetGlobalIDPacket) != globalIDPacketList.end())
            {
                Log alreadyBeacon("Node " + std::to_string(nodeId) + " already received this beacon, dropping", true);
                logger.logMessage(alreadyBeacon);
                // drop Message
                dropAnimationPacket dropPacket(nodeId);
                sf::Packet dropPacketReceiver;
                dropPacketReceiver << dropPacket;
                logger.sendTcpPacket(dropPacketReceiver);
            }
            else
            {
                // it's a new beacon, we reenter boradcast mode regardless of the beacons left to send
                globalIDPacketList.push_back(packetGlobalIDPacket);
                shouldSendBeacon = true; // at next transmission slots, will create the new sending beacon scheduler
                beaconSlots.clear();
            }
        }

        return true; // it's for the compiler to not throw a warning, we never capture this variable
    }
}

//----------------------------STATE TRANSITIONS--------------------------------

bool C2_Node::canSleepFromCommunicating()
{ // Node Can alwasy sleep
    sf::Packet statePacketReceiver;
    stateNodePacket statePacket(nodeId, "Sleep");
    statePacketReceiver << statePacket;
    logger.sendTcpPacket(statePacketReceiver);
    //Todo: Protect with mutexes.

    currentState = NodeState::Sleeping;
    // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
    // logger.logMessage(transitionLog);
    return true;
}

bool C2_Node::canCommunicateFromSleeping()
{
    // TODO: put these States "Communicate", "Sleep" in Common.hpp to have a common reference !!
    sf::Packet statePacketReceiver;
    stateNodePacket statePacket(nodeId, "Communicate");
    statePacketReceiver << statePacket;
    logger.sendTcpPacket(statePacketReceiver);

    isTransmittingWhileCommunicating = false;
        //Todo: Protect with mutexes.

    currentState = NodeState::Communicating;
    if (shouldSendBeacon && beaconSlots.size() == 0)
    {
        // a "new" beacon has just been received, we plan the random slots
        shouldSendBeacon = false;
        beaconSlots = selectRandomSlots(computeRandomNbBeaconPackets(common::minimumNbBeaconPackets, common::maximumNbBeaconPackets), common::nbSlotsPossibleForOneBeacon);
        std::ostringstream oss;
        for (size_t i = 0; i < beaconSlots.size(); ++i)
        {
            oss << beaconSlots[i];
            if (i < beaconSlots.size() - 1)
            {
                oss << ", "; // Add a separator between elements
            }
        }
        // Log beaconSlotsLog("Node "+std::to_string(nodeId)+" will send beacons at slots: "+oss.str(), true);
        // logger.logMessage(beaconSlotsLog);
    }
    if (beaconSlots.size() > 0)
    {
        // we have beacons to send
        if (beaconSlots[0] == 0)
        {
            isTransmittingWhileCommunicating = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

            // create the beacon packet
            std::vector<uint8_t> beaconPacket;
            // preallocate the space for optimization
            beaconPacket.reserve(common::typeBytesSize + common::timeStampBytesSize + common::costFunctionBytesSize +
                                 common::hopCountBytesSize + common::globalIDPacketBytesSize +
                                 common::senderGlobalIdBytesSize + common::hashFunctionBytesSize);

            // prepare the fields
            std::vector<uint8_t> newTimeStamp = getTimeStamp();
            std::vector<uint8_t> newCostFunction = {pathCost.value()};
            std::vector<uint8_t> newHopCount = decimalToBytes(hopCount.value(), common::hopCountBytesSize);
            std::vector<uint8_t> newGlobalIDPacket = decimalToBytes(globalIDPacketList.back(), common::globalIDPacketBytesSize);
            std::vector<uint8_t> newSenderGlobalId = decimalToBytes(nodeId, common::senderGlobalIdBytesSize);
            std::vector<uint8_t> newHashFunction = {0x00, 0x00, 0x00, 0x00};

            // Append all fields
            appendVector(beaconPacket, common::type);
            appendVector(beaconPacket, newTimeStamp);
            appendVector(beaconPacket, newCostFunction);
            appendVector(beaconPacket, newHopCount);
            appendVector(beaconPacket, newGlobalIDPacket);
            appendVector(beaconPacket, newSenderGlobalId);
            appendVector(beaconPacket, newHashFunction);

            sf::Packet broadcastPacketReceiver;
            broadcastMessagePacket broadcastPacket(nodeId);
            broadcastPacketReceiver << broadcastPacket;
            logger.sendTcpPacket(broadcastPacketReceiver);

            addMessageToTransmit(beaconPacket, std::chrono::milliseconds(common::timeOnAirBeacon));
            beaconSlots.erase(beaconSlots.begin());
            // TODO: we should simulate the node is transmitting here, right now it's instantaneous
        }
        if (!beaconSlots.empty())
        {
            // decrease every elements of the slots by one
            for (int i = 0; i < beaconSlots.size(); i++)
            {
                beaconSlots[i]--;
            }
        }
    }
    return true;
}

// Unauthorized transition in this mode.
bool C2_Node::canCommunicateFromTransmitting() { return false; }
bool C2_Node::canCommunicateFromListening() { return false; }
bool C2_Node::canCommunicateFromCommunicating() { return true; }
bool C2_Node::canTransmitFromListening() { return true; }
bool C2_Node::canTransmitFromSleeping() { return true; }
bool C2_Node::canTransmitFromTransmitting() { return true; }
bool C2_Node::canTransmitFromCommunicating() { return false; }
bool C2_Node::canListenFromTransmitting() { return true; }
bool C2_Node::canListenFromSleeping() { return false; }
bool C2_Node::canListenFromListening() { return false; }
bool C2_Node::canListenFromCommunicating() { return false; }
bool C2_Node::canSleepFromTransmitting() { return false; }
bool C2_Node::canSleepFromListening() { return false; }
bool C2_Node::canSleepFromSleeping() { return false; }

#elif COMMUNICATION_PERIOD == RRC_DOWNLINK

bool C2_Node::canNodeReceiveMessage()
{
        //Todo: Protect with mutexes.

    // State Condition: node must be listening to receive a message
    if (currentState != NodeState::Listening && currentState != NodeState::Communicating)
    {
        return false;
    }
    else if (currentState == NodeState::Communicating)
    {
        return !isTransmittingWhileCommunicating;
    }
    return true;
}

bool C2_Node::receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir)
{
    if (!canNodeReceiveMessage())
    {
        Log notlisteninglog("Node " + std::to_string(nodeId) + " not listening, dropped msg" /*+detailedBytesToString( message)*/, true);
        logger.logMessage(notlisteninglog);

        sf::Packet receptionStatePacketReceiver;
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
        receiveMessagePacket receptionState(senderId, nodeId, "notListening");
        receptionStatePacketReceiver << receptionState;
        logger.sendTcpPacket(receptionStatePacketReceiver);

        return false;
    }

    if (!Node::receiveMessage(message, timeOnAir))
    {
        // an interference happened, we don't treat the message
        sf::Packet receptionStatePacketReceiver;
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
        receiveMessagePacket receptionState(senderId, nodeId, "interference");
        receptionStatePacketReceiver << receptionState;
        logger.sendTcpPacket(receptionStatePacketReceiver);

        return false;
    }

    uint8_t type = extractBytesFromField(message, "type", common::fieldMap);
    if (type != common::type[0])
    {
        // not a beacon, we don't care
        Log wrongTypeLog("Node " + std::to_string(nodeId) + " received Incorrecty packet type, dropping", true);
        logger.logMessage(wrongTypeLog);
        // receiveBuffer.pop();
        return false;
    }

    sf::Packet receptionStatePacketReceiver;
    uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
    receiveMessagePacket receptionState(senderId, nodeId, "received");
    receptionStatePacketReceiver << receptionState;
    logger.sendTcpPacket(receptionStatePacketReceiver);

    uint8_t packetSenderId = extractBytesFromField(message, "senderGlobalId", common::fieldMap);
    uint32_t packetReceiverId = extractBytesFromField(message, "receiverGlobalId", common::fieldMap);
    uint16_t packetGlobalIDPacket = extractBytesFromField(message, "globalIDPacket", common::fieldMap);
    uint8_t packetPayload = extractBytesFromField(message, "payload", common::fieldMap);
    uint8_t packetHashFunction = extractBytesFromField(message, "hashFunction", common::fieldMap);
    // //TODO: remove the buffer?? maybe it's just used by the simulation manager
    // receiveBuffer.pop();//we don't care about the other messages
    // //actually, we don't really care about the receiving buffer, since each message is treated as soon as it is received.

    if (packetReceiverId == nodeId)
    { // add C1 Childs condition when you implement more complex topology
        // THe packet is for us!
        Log finalReceiverLog("Node " + std::to_string(nodeId) + " received a packet for him", true);
        logger.logMessage(finalReceiverLog);
        // TODO: make clear in the visualiser that there is something hapenning?
        // woud be nice you reuse what you implemented, and don't make another packet

        // maybe put something to signify in the visualiser that It Works TODO
        return true;
    }

    std::lock_guard<std::mutex> lock(receiveMutex);
    if (globalIDPacketList.empty())
    {
        // I was lazy so I copy past code from beacon mode, it's relatively the same, hence the disturbing name variables TODO change that
        // this is the first beacon received
        shouldSendBeacon = true; // next tranmission slots, will create the new sending beacon scheduler
        globalIDPacketList.push_back(packetGlobalIDPacket);
        packetFinalReceiverId = packetReceiverId;
        return true;
    }
    else
    {

        if (std::find(globalIDPacketList.begin(), globalIDPacketList.end(), packetGlobalIDPacket) != globalIDPacketList.end())
        {
            Log alreadyBeacon("Node " + std::to_string(nodeId) + " already received this beacon, dropping", true);
            logger.logMessage(alreadyBeacon);
            // drop Message
            dropAnimationPacket dropPacket(nodeId);
            sf::Packet dropPacketReceiver;
            dropPacketReceiver << dropPacket;
            logger.sendTcpPacket(dropPacketReceiver);
        }
        else
        {
            // this is another downlink message. The node stops sending former messages and restars with new
            globalIDPacketList.push_back(packetGlobalIDPacket);
            shouldSendBeacon = true; // at next transmission slots, will create the new sending beacon scheduler
            packetFinalReceiverId = packetReceiverId;

            // maybe don't clear if you want to keep the former messages to be sent yet
            beaconSlots.clear();
        }
    }

    return true; // it's for the compiler to not throw a warning, we never capture this variable
}

//----------------------------STATE TRANSITIONS--------------------------------

bool C2_Node::canCommunicateFromSleeping()
{

    sf::Packet statePacketReceiver;
    stateNodePacket statePacket(nodeId, "Communicate");
    statePacketReceiver << statePacket;
    logger.sendTcpPacket(statePacketReceiver);

    isTransmittingWhileCommunicating = false;
        //Todo: Protect with mutexes.

    currentState = NodeState::Communicating;
    if (shouldSendBeacon && beaconSlots.size() == 0)
    {
        // a "new" beacon has just been received, we plan the random slots
        shouldSendBeacon = false;
        beaconSlots = selectRandomSlots(computeRandomNbBeaconPackets(common::minimumNbBeaconPackets, common::maximumNbBeaconPackets), common::nbSlotsPossibleForOneBeacon);
        std::ostringstream oss;
        for (size_t i = 0; i < beaconSlots.size(); ++i)
        {
            oss << beaconSlots[i];
            if (i < beaconSlots.size() - 1)
            {
                oss << ", "; // Add a separator between elements
            }
        }
        // Log beaconSlotsLog("Node "+std::to_string(nodeId)+" will send beacons at slots: "+oss.str(), true);
        // logger.logMessage(beaconSlotsLog);
    }
    if (beaconSlots.size() > 0)
    {
        // we have beacons to send
        if (beaconSlots[0] == 0)
        {
            isTransmittingWhileCommunicating = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

            //-------------------------------------define Beacon Packet----------------------------
            //----------least signficant byte first (little endian) !------------
            std::vector<uint8_t> globalIDPacket = decimalToBytes(globalIDPacketList.back(), common::globalIDPacketBytesSize); // Global ID is 2 byte long in the simulation, 10 bits in real life
            std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId, 2);                                                  // Sender Global ID is 2 byte long in the simulation, 10 bits in real life

            std::vector<uint8_t> receiverGlobalId = decimalToBytes(packetFinalReceiverId.value(), 2); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
            std::vector<uint8_t> payload = {0xFF, 0xFF, 0xFF, 0xFF};                                  // Payload Size is 4 byte long in the simulation, 40 Bytes max in real life

            // dummy hash: we don't implement the hash function in this simulation
            std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00}; // Hash Function is 4 byte long in the simulation AND in real life

            // Concatenate fields into one vector
            std::vector<uint8_t> beaconPacket;

            // preallocate the space for optimization
            // TODO: should use the size in the common file, not the variable, source of error
            beaconPacket.reserve(common::type.size() +
                                 senderGlobalId.size() +
                                 receiverGlobalId.size() +
                                 globalIDPacket.size() +
                                 payload.size() +
                                 +hashFunction.size());

            // Append all fields
            appendVector(beaconPacket, common::type);
            appendVector(beaconPacket, senderGlobalId);
            appendVector(beaconPacket, receiverGlobalId);
            appendVector(beaconPacket, globalIDPacket);
            appendVector(beaconPacket, payload);
            appendVector(beaconPacket, hashFunction);

            sf::Packet broadcastPacketReceiver;
            broadcastMessagePacket broadcastPacket(nodeId);
            broadcastPacketReceiver << broadcastPacket;
            logger.sendTcpPacket(broadcastPacketReceiver);

            addMessageToTransmit(beaconPacket, std::chrono::milliseconds(common::timeOnAirFlood));
            beaconSlots.erase(beaconSlots.begin());
        }
        if (!beaconSlots.empty())
        {
            // decrease every elements of the slots by one
            for (int i = 0; i < beaconSlots.size(); i++)
            {
                beaconSlots[i]--;
            }
        }
    }
    return true;
}

bool C2_Node::canSleepFromCommunicating()
{ // Node Can alwasy sleep

    sf::Packet statePacketReceiver;
    stateNodePacket statePacket(nodeId, "Sleep");
    statePacketReceiver << statePacket;
    logger.sendTcpPacket(statePacketReceiver);

    //Todo: Protect with mutexes.
    currentState = NodeState::Sleeping;
    // Log transitionLog("Node "+std::to_string(nodeId)+" sleeps", true);
    // logger.logMessage(transitionLog);
    return true;
}
// Unauthorized transition in this mode.
bool C2_Node::canCommunicateFromTransmitting() { return false; }
bool C2_Node::canCommunicateFromListening() { return false; }
bool C2_Node::canCommunicateFromCommunicating() { return true; }
bool C2_Node::canTransmitFromListening() { return true; }
bool C2_Node::canTransmitFromSleeping() { return true; }
bool C2_Node::canTransmitFromTransmitting() { return true; }
bool C2_Node::canTransmitFromCommunicating() { return false; }
bool C2_Node::canListenFromTransmitting() { return true; }
bool C2_Node::canListenFromSleeping() { return false; }
bool C2_Node::canListenFromListening() { return false; }
bool C2_Node::canListenFromCommunicating() { return false; }
bool C2_Node::canSleepFromTransmitting() { return false; }
bool C2_Node::canSleepFromListening() { return false; }
bool C2_Node::canSleepFromSleeping() { return false; }

#elif COMMUNICATION_PERIOD == RRC_UPLINK

// Display------------------------------------------------------------------------------------------------------



void C2_Node::displayRouting()
{
    sf::Packet routingPacketReceiver;
    routingDecisionPacket routingPacket(nodeId, infoFromBeaconPhase.getNextNodeIdInPath(), true);
    routingPacketReceiver << routingPacket;
    logger.sendTcpPacket(routingPacketReceiver);

    // Log rootingLog("Node "+std::to_string(nodeId)+" rooting with Node:"+std::to_string(nextNodeIdInPath.value()), true);
    // logger.logMessage(rootingLog);
}

// END - Display------------------------------------------------------------------------------------------------------

void C2_Node::buildAndTransmitDataPacket(std::vector<uint8_t> payload = {})
{

    // receiving window are greater than transmitting window, so we make the transmitting node waiting the guard time
    std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

    // create the data packet
    std::vector<uint8_t> dataPacket;

    // preallocate the space for optimization
    dataPacket.reserve(common::typeData.size() +
                       common::senderGlobalIdBytesSize +
                       common::receiverGlobalIdBytesSize +
                       common::localIDPacketBytesSize +
                       common::payloadSizeBytesSize +
                       common::hashFunctionBytesSize);

    // prepare the fields
    std::vector<uint8_t> senderGlobalId = decimalToBytes(nodeId, common::senderGlobalIdBytesSize);                                        // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> receiverGlobalId = decimalToBytes(infoFromBeaconPhase.getNextNodeIdInPath(), common::receiverGlobalIdBytesSize); // Receiver Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> localIDPacket = decimalToBytes(localIDPacketCounter, common::localIDPacketBytesSize);                            // we increase the counter if we receive the ACK

    // Should be replaced by the parameter payload.
    std::vector<uint8_t> payloadPacket = {0xFF, 0xFF, 0xFF, 0xFF}; // Payload Size is 4 byte long in the simulation, 40 Bytes max in real life
    std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00};  // Hash Function is 4 byte long in the simulation AND in real life

    // Append all fields

    appendVector(dataPacket, common::typeData);
    appendVector(dataPacket, senderGlobalId);
    appendVector(dataPacket, receiverGlobalId);
    appendVector(dataPacket, localIDPacket);
    appendVector(dataPacket, payloadPacket);
    appendVector(dataPacket, hashFunction);

    // virtualization of the Transmit() from Physical Layer
    addMessageToTransmit(dataPacket, std::chrono::milliseconds(common::timeOnAirDataPacket));

    // same logic, even though it is not necessary in the simualator
    std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));
}

void C2_Node::buildAndTransmitAckPacket()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));

    std::vector<uint8_t> ackPacket;

    // preallocate the space for optimization
    ackPacket.reserve(common::typeACK.size() +
                      common::senderGlobalIdBytesSize +
                      common::receiverGlobalIdBytesSize +
                      common::localIDPacketBytesSize +
                      common::hashFunctionBytesSize);

    // prepare the fields:
    std::vector<uint8_t> senderGlobalIdPacket = decimalToBytes(nodeId, common::senderGlobalIdBytesSize); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life

    auto ackInformationIds = ackInformation.getAndResetAckInformation();
    std::vector<uint8_t> receiverGlobalIdPacket = decimalToBytes(ackInformationIds.first, common::receiverGlobalIdBytesSize); // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> localIDPacket = decimalToBytes(ackInformationIds.second, common::localIDPacketBytesSize);            // Sender Global ID is 2 byte long in the simulation, 10 bits in real life
    std::vector<uint8_t> hashFunction = {0x00, 0x00, 0x00, 0x00};                                                             // Hash Function is 4 byte long in the simulation AND in real life

    // Append all fields
    appendVector(ackPacket, common::typeACK);
    appendVector(ackPacket, senderGlobalIdPacket);
    appendVector(ackPacket, receiverGlobalIdPacket);
    appendVector(ackPacket, localIDPacket);
    appendVector(ackPacket, hashFunction);
    std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));
    addMessageToTransmit(ackPacket, std::chrono::milliseconds(common::timeOnAirAckPacket));

    adressedPacketTransmissionDisplay(ackInformationIds.first,true);

    std::this_thread::sleep_for(std::chrono::milliseconds(common::guardTime));
}

// Receive------------------------------------------------------------------------------------------------------
bool C2_Node::canNodeReceiveMessage()
{
    NodeState currentStatelocal = getCurrentState();
    // State Condition: node must be listening to receive a message
    if (currentStatelocal != NodeState::Listening && currentStatelocal != NodeState::Communicating)
    {
        return false;
    }
    // They also need to NOT be actively transmitting
    else if (currentStatelocal == NodeState::Communicating)
    {
        return !isTransmittingWhileCommunicating;
    }
    return true;
}


void C2_Node::handleDataPacketReception(const std::vector<uint8_t> &message, uint16_t senderId, uint32_t packetId)
{

    // we store the local packet ID in map. This is how we know if the packet is new or not
    auto &packetList = packetsMap[senderId]; // Get the vector for the sender
    // Only add if the packet is not already present
    if (std::find(packetList.begin(), packetList.end(), packetId) == packetList.end())
    {
        packetList.push_back(packetId);
        // TODO: it's a simulation, in real implementation, we should save the message and add it to a buffer
        nbPayloadLeft++;
    }
}

void C2_Node::handleAckPacketReception(uint16_t senderId, uint32_t packetId)
{
    // is it the Ack packet we were waiting for?
    if (packetId == localIDPacketCounter)
    {
        // we received the ACK for the last packet we sent
        nbPayloadLeft--;        // todo:in real life, we remove the payload from the buffer
        localIDPacketCounter++; // increasing the counter to indicate we send a "new" packet
        retransmissionCounterHelper.setIsExpectingAck(false);
        receptionStateDisplay(senderId, "received");
    }
    else
    {
        dropAnimationDisplay();
    }
}

bool C2_Node::receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir)
{

    // Node must listen/communicate and not transmit  to receive a message
    if (!canNodeReceiveMessage())
    {
        // Log notlisteninglog("Node " + std::to_string(nodeId) + " not listening, dropped msg" /*+detailedBytesToString( message)*/, true);
        // logger.logMessage(notlisteninglog);
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::dataFieldMap);
        receptionStateDisplay(senderId, "notListening");
        return false;
    }

    // there should be no interference
    else if (!Node::receiveMessage(message, timeOnAir))
    {
        // an interference happened, we don't treat the message
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::dataFieldMap);
        receptionStateDisplay(senderId, "interference");
        return false;
    }
    // The packet must be adressed to us to be processed
    // TODO: put the name of the fields in the common file, or in a struct
    uint16_t receiverId = extractBytesFromField(message, "receiverGlobalId", common::dataFieldMap);

    // if packet is not for this node,message is dropped
    if (receiverId != nodeId)
    {
        // Log wrongReceiverLog("Node " + std::to_string(nodeId) + " received a packet not for him, dropping", true);
        // logger.logMessage(wrongReceiverLog);
        dropAnimationDisplay();
        return false;
    }

    // is it a data packet and  are we in a data window
    if (message[0] == common::typeData[0] && !isACKSlot)
    {
        // we received a packet for us, we should send an ack no matter what happened before (ack can be lost so we should not check if we already sent one)
        auto lastLocalIdPacket = extractBytesFromField(message, "localIDPacket", common::dataFieldMap);
        auto lastSenderId = extractBytesFromField(message, "senderGlobalId", common::dataFieldMap);
        ackInformation.setNewAckInformation(lastSenderId, lastLocalIdPacket);
        handleDataPacketReception(message, lastSenderId, lastLocalIdPacket);
        // Indicate the visualiser the packet is received and handled
        receptionStateDisplay(lastSenderId, "received");
    }
    else if (message[0] == common::typeACK[0] && isACKSlot)
    {
        // it's an ACK packet and we are in an ACK window
        uint16_t localIdPacket = extractBytesFromField(message, "localIDPacket", common::dataFieldMap);
        uint16_t senderId = extractBytesFromField(message, "senderGlobalId", common::dataFieldMap);
        // Visualiser Display behavior inside the below function.
        handleAckPacketReception(senderId, localIdPacket);
    }

    // message has been handled correctly, even the case if data packet received in not a data window??? to check
    return true;
}
// End -  Receive------------------------------------------------------------------------------------------------------

// State Transitions ---------------------------------------------------------------------------------
bool C2_Node::canCommunicateFromSleeping()
{

    // Todo: should be put into the constructor but doesn´t work, probably an optionnal not being initialized?
    //  the first state transition, we display rooting in the visualiser if applicable
    if (common::visualiserConnected)
    {
        if (!routingDisplayed)
        {
            displayRouting();
            routingDisplayed = true;
        }
    }


    isACKSlot = !isACKSlot; // switch to new slot category everytime we enter a new communication window (Data or ACK slot)
    
    // This is a physical layer property that is updated in the network layer, not good !!! todo
    isTransmittingWhileCommunicating = false; // used to detect when node cannot physically receive messages because they transmit
  
    if(!isACKSlot)     //Advance the global data slot category (mod 3) (part of the protocol) for the next iteration
    currentDataSlotCategory =( (currentDataSlotCategory + 1) % 3);

    // Change of state is only allowed if the current DataSlotCategory is corresponding to the HopCountmodulo (the node might transmit info) 
    //or the HopCOuntmodulo+1 (the node might receive ACK)
    bool showDisplay = true;
    if( currentDataSlotCategory != fixedSlotCategory && currentDataSlotCategory != ((fixedSlotCategory+1)%3))
    {
        //TODO
        // showDisplay = false; //garbage code bc there is a bug I cannot solve when returning false here
                             //we do that to avoid counting Energy Expenditure when nodes should be sleeping instead of listening.
   return false;
    }



    
    if (!isACKSlot)
    {

        if(showDisplay) nodeStateDisplay("Communicate", false);
        handleDataSlotPhase();
    }
    else
    {

        if(showDisplay) nodeStateDisplay("Communicate", true);
        handleAckSlotPhase();
    }

    // if(!showDisplay) //if we are not in the right slot category, we do not change the state
    // {
    //     return false;
    // }
    // we can communicate
    setCurrentState(NodeState::Communicating);
    return true;
}

bool C2_Node::canSleepFromCommunicating()
{
    retransmissionCounterHelper.toggleSecondSleepWindow();

    if (retransmissionCounterHelper.getIsExpectingAck() && retransmissionCounterHelper.getSecondSleepWindow())
    {
        // we sent a Data packet and didn't receive an ACK in the next transmission window, so there will be retransmission-> specific metric in the vizualiser.
        retransmissionPacket retransmissionPacket(nodeId);
        sf::Packet retransmissionPacketReceiver;
        retransmissionPacketReceiver << retransmissionPacket;
        logger.sendTcpPacket(retransmissionPacketReceiver);
        retransmissionCounterHelper.setIsExpectingAck(false);
    }

    setCurrentState( NodeState::Sleeping);
    nodeStateDisplay("Sleep",std::nullopt);
    return true;
}

//A node can refuse to change to communicate state if it is not in the right slot category
bool C2_Node::canSleepFromSleeping() { return true; }


// Unauthorized transition in this mode.
bool C2_Node::canCommunicateFromTransmitting() { return false; }
bool C2_Node::canCommunicateFromListening() { return false; }
bool C2_Node::canCommunicateFromCommunicating() { return false; }
bool C2_Node::canTransmitFromListening() { return false; }
bool C2_Node::canTransmitFromSleeping() { return false; }
bool C2_Node::canTransmitFromTransmitting() { return false; }
bool C2_Node::canTransmitFromCommunicating() { return false; }
bool C2_Node::canListenFromTransmitting() { return true; }
bool C2_Node::canListenFromSleeping() { return false; }
bool C2_Node::canListenFromListening() { return false; }
bool C2_Node::canListenFromCommunicating() { return false; }
bool C2_Node::canSleepFromTransmitting() { return false; }
bool C2_Node::canSleepFromListening() { return false; }

// End - State Transitions ---------------------------------------------------------------------------------

// Slot Strategy ----------------------------------------------------------------------------------
void C2_Node::handleAckSlotPhase()
{
    if (ackInformation.shouldReplyAck())
    {
        if(nodeId==9){
            int a=3;
        }
        // we have an ACK to send
        isTransmittingWhileCommunicating = true;
        buildAndTransmitAckPacket();
    }


}

void C2_Node::handleDataSlotPhase()
{
    // Node can only act when its fixed category matches the current simulation slot category
    if (fixedSlotCategory == currentDataSlotCategory)
    {

        if (slotManager.canTransmitNow() && nbPayloadLeft > 0)
        {
            isTransmittingWhileCommunicating = true;
            slotManager.consumeSlot();

            buildAndTransmitDataPacket();
            retransmissionCounterHelper.setIsExpectingAck(true);

            adressedPacketTransmissionDisplay(infoFromBeaconPhase.getNextNodeIdInPath(),false);
        }
         slotManager.decrementAllSlots();
    }
}

// End - Slot Strategy -------------------------------------------------------------------------------

#else
#error "Unknown COMMUNICATION_PERIOD mode"
#endif