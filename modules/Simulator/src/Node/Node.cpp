#include "Node.hpp"
#include "../Connectivity/TCP/Packets/Packets.hpp"
#include "../PhyLayer/PhyLayer.hpp"
// if this becomes too messy, think about creating an object to populate the node
Node::Node(int id, Logger &logger, std::pair<int, int> coordinates, double batteryLevel)
    : nodeId(id), logger(logger), coordinates(coordinates), batteryLevel(batteryLevel)
{
   }

std::string Node::initMessage() const
{
    return "Node " + std::to_string(nodeId) + " located at (" + std::to_string(coordinates.first) + "," + std::to_string(coordinates.second) + ")";
}



void Node::initializeTransitionMap()
{
    // Safe to assign the C1/2/3 class's callBack function after construction
    // this function will be called in the constructor of the Child classes ! (C1, C2, C3), otherwise virtual func implementation cannot be solved
    // state transition name convention: Proposed state -> Current state -> Condition check function

    stateTransitions[{WindowNodeState::CanTransmit, NodeState::Listening}] = [this]()
    { return canTransmitFromListening(); };
    stateTransitions[{WindowNodeState::CanTransmit, NodeState::Transmitting}] = [this]()
    { return canTransmitFromTransmitting(); };
    stateTransitions[{WindowNodeState::CanTransmit, NodeState::Sleeping}] = [this]()
    { return canTransmitFromSleeping(); };
    stateTransitions[{WindowNodeState::CanTransmit, NodeState::Communicating}] = [this]()
    { return canTransmitFromCommunicating(); };

    stateTransitions[{WindowNodeState::CanListen, NodeState::Transmitting}] = [this]()
    { return canListenFromTransmitting(); };
    stateTransitions[{WindowNodeState::CanListen, NodeState::Listening}] = [this]()
    { return canListenFromListening(); };
    stateTransitions[{WindowNodeState::CanListen, NodeState::Sleeping}] = [this]()
    { return canListenFromSleeping(); };
    stateTransitions[{WindowNodeState::CanListen, NodeState::Communicating}] = [this]()
    { return canListenFromCommunicating(); };

    stateTransitions[{WindowNodeState::CanSleep, NodeState::Transmitting}] = [this]()
    { return canSleepFromTransmitting(); };
    stateTransitions[{WindowNodeState::CanSleep, NodeState::Listening}] = [this]()
    { return canSleepFromListening(); };
    stateTransitions[{WindowNodeState::CanSleep, NodeState::Sleeping}] = [this]()
    { return canSleepFromSleeping(); };
    stateTransitions[{WindowNodeState::CanSleep, NodeState::Communicating}] = [this]()
    { return canSleepFromCommunicating(); };

    stateTransitions[{WindowNodeState::CanCommunicate, NodeState::Transmitting}] = [this]()
    { return canCommunicateFromTransmitting(); };
    stateTransitions[{WindowNodeState::CanCommunicate, NodeState::Listening}] = [this]()
    { return canCommunicateFromListening(); };
    stateTransitions[{WindowNodeState::CanCommunicate, NodeState::Sleeping}] = [this]()
    { return canCommunicateFromSleeping(); };
    stateTransitions[{WindowNodeState::CanCommunicate, NodeState::Communicating}] = [this]()
    { return canCommunicateFromCommunicating(); };
}

void Node::adressedPacketTransmissionDisplay(uint16_t receiverId, bool isAck) const
{
    sf::Packet transmitPacketReceiver;
    transmitMessagePacket transmitPacket(nodeId, receiverId, isAck);
    transmitPacketReceiver << transmitPacket;
    logger.sendTcpPacket(transmitPacketReceiver);
}
void Node::endAdressedPacketTransmissionDisplay(uint16_t receiverId) const
{
    sf::Packet endTransmitPacketReceiver;
    endTransmissionPacket endPacket(nodeId, receiverId);
    endTransmitPacketReceiver << endPacket;
    logger.sendTcpPacket(endTransmitPacketReceiver);
}
void Node::receptionStateDisplay(uint16_t senderId, ReceptionState state)
{
    sf::Packet receptionStatePacketReceiver;

    std::string stateStr;
    switch (state)
    {
    case ReceptionState::Interference:
        stateStr = "Interference";
        break;
    case ReceptionState::NotListening:
        stateStr = "NotListening";
        break;
    case ReceptionState::Received:
        stateStr = "Received";
        break;
    default:
        stateStr = "unknown";
    }
    receiveMessagePacket receptionState(senderId, nodeId, stateStr);
    receptionStatePacketReceiver << receptionState;
    logger.sendTcpPacket(receptionStatePacketReceiver);
}

void Node::dropAnimationDisplay()
{
    dropAnimationPacket dropPacket(nodeId);
    sf::Packet dropPacketReceiver;
    dropPacketReceiver << dropPacket;
    logger.sendTcpPacket(dropPacketReceiver);
}

void Node::nodeStateDisplay(std::string state, std::optional<bool> isCommunicatingAck)
{
    sf::Packet statePacketReceiver;
    stateNodePacket statePacket(nodeId, state);

    if (isCommunicatingAck.has_value())
    {
        statePacket.isCommunicatingAck = isCommunicatingAck;
    }

    statePacketReceiver << statePacket;
    logger.sendTcpPacket(statePacketReceiver);
}


NodeState Node::convertWindowNodeStateToNodeState(WindowNodeState state)
{
    switch (state)
    {
    case WindowNodeState::CanCommunicate:
        return NodeState::Communicating;
    case WindowNodeState::CanTransmit:
        return NodeState::Transmitting;
    case WindowNodeState::CanListen:
        return NodeState::Listening;
    case WindowNodeState::CanSleep:
        return NodeState::Sleeping;
    default:
        throw std::invalid_argument("Invalid WindowNodeState for conversion");
    }
}

void Node::onTimeChange(WindowNodeState proposedState) {
    auto key = std::make_pair(proposedState, currentState);

    auto it = stateTransitions.find(key);
    if (it != stateTransitions.end()) {
        if (it->second()) {
            // Transition succeeded - schedule next one
            scheduleNextTransition();
        } else {
            logEvent("Failed Transition: " + stateToString(currentState) + 
                    " to " + windowStateToString(proposedState));
        }
    } else {
        logEvent("No state transition rule found: from " + stateToString(currentState) + 
                " to " + windowStateToString(proposedState));
    }
}


// simulate the reception of a message, including potential interferences
bool Node::receiveMessage(const std::vector<uint8_t> message)
{

    return false;
}

void Node::addMessageToTransmit(const std::vector<uint8_t>& message, int64_t airtimeMs)
{
    if (phyLayer == nullptr) {
        throw std::runtime_error("PhyLayer not set for Node " + std::to_string(nodeId));
    }
    phyLayer->processTransmission(this, message, airtimeMs);
}





void Node::setScheduleBlueprint(std::shared_ptr<IScheduleBlueprint> bp) {
    if (!bp) {
        throw std::invalid_argument("Cannot set null blueprint");
    }
    blueprint = bp;
}

int64_t Node::getCurrentTime() const {
    if (!phyLayer) {
        throw std::runtime_error("PhyLayer not set for Node " + std::to_string(nodeId));
    }
    return phyLayer->getClock()->currentTimeInMilliseconds();
}

void Node::scheduleNextTransition() {
    if (!blueprint) {
        throw std::runtime_error("Node " + std::to_string(nodeId) + " has no blueprint set");
    }
    
    if (!phyLayer) {
        throw std::runtime_error("PhyLayer not set for Node " + std::to_string(nodeId));
    }
    
    int64_t currentTime = getCurrentTime();
    
    // Try to get next transition from blueprint
    auto nextTime = blueprint->tryGetNextTransitionTime(currentTime, nodeClass);
    
    if (!nextTime.has_value()) {
        // No pattern defined for current mode - node is in free scheduling mode
        logEvent("Entering free scheduling mode - no pattern defined");
        return;
    }
    
    // Get the state at the next transition time
    auto nextState = blueprint->tryGetStateAt(*nextTime, nodeClass);
    
    if (!nextState.has_value()) {
        throw std::runtime_error("Blueprint returned transition time but no state - inconsistent");
    }
    
    // Schedule the transition via PhyLayer's clock
    phyLayer->scheduleStateTransition(*nextTime, [this, nextState = *nextState]() {
        this->onTimeChange(nextState);
    });
    // Schedule handleCommunication() at the same time
    phyLayer->getClock()->scheduleCommunicationStep(*nextTime, shared_from_this());
}

void Node::scheduleCustomTransition(int64_t absoluteTime, WindowNodeState state) {
    if (!phyLayer) {
        throw std::runtime_error("PhyLayer not set for Node " + std::to_string(nodeId));
    }
    
    if (absoluteTime <= getCurrentTime()) {
        throw std::invalid_argument("Cannot schedule transition in the past or at current time");
    }
    
    phyLayer->scheduleStateTransition(absoluteTime, [this, state]() {
        this->onTimeChange(state);
    });
    
    logEvent("Custom transition scheduled at " + std::to_string(absoluteTime) + 
             " to " + windowStateToString(state));
}

