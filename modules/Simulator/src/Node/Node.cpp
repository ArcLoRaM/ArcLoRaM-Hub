#include "Node.hpp"
#include "../Connectivity/TCP/packets.hpp"
#include "../PhyLayer/PhyLayer.hpp"
// if this becomes too messy, think about creating an object to populate the node
Node::Node(int id, Logger &logger, std::pair<int, int> coordinates, double batteryLevel)
    : nodeId(id), running(true), logger(logger), coordinates(coordinates), batteryLevel(batteryLevel)
{
   }

std::string Node::initMessage() const
{
    // retrieve the thread id
    std::thread::id this_id = std::this_thread::get_id();
    std::stringstream ss;
    ss << this_id;
    std::string threadIdStr = ss.str();

    return "Node " + std::to_string(nodeId) + " located at (" + std::to_string(coordinates.first) + "," + std::to_string(coordinates.second) + ")";
}

std::string toString(NodeState state) {
    switch (state) {
        case NodeState::Transmitting:   return "Transmitting";
        case NodeState::Listening:      return "Listening";
        case NodeState::Sleeping:       return "Sleeping";
        case NodeState::Communicating:  return "Communicating";
        default:                        return "Unknown";
    }
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
void Node::receptionStateDisplay(uint16_t senderId, std::string state)
{
    // TODO: put a common enum for the possible states

    sf::Packet receptionStatePacketReceiver;
    receiveMessagePacket receptionState(senderId, nodeId, state);
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
const std::vector<std::pair<int64_t, WindowNodeState>>& Node::getActivationSchedule() const {
    return activationSchedule;
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

void Node::addActivation(int64_t activationTime, WindowNodeState activationState)
{
    // add a new activation time and state to the schedule
    activationSchedule.emplace_back(activationTime, activationState);
}

void Node::onTimeChange(WindowNodeState proposedState)
{

  
        auto key = std::make_pair(proposedState, currentState);

        // Check if there is a registered transition function for the proposed and current state
        auto it = stateTransitions.find(key);
        if (it != stateTransitions.end()) {

            // Call the transition function (condition function)
            if (it->second()) {
                
             } else {
                logEvent("Failed Transition:"+stateToString(currentState)+" to "+stateToString(proposedState) );
                //logger.logMessage(failedTransitionLog);
            }
        } else {
            logEvent("No state transition rule found: from " + stateToString(currentState) + " to " + stateToString(proposedState));
        } 
        
}

// todo: wiht Magic Enum, you dont need this anymore
std::string Node::stateToString(NodeState state)
{
    switch (state)
    {
    case NodeState::Communicating:
        return " Communicating";
    case NodeState::Transmitting:
        return " Transmiting";
    case NodeState::Listening:
        return " Listening";
    case NodeState::Sleeping:
        return " Sleeping";
    default:
        return "Unknown";
    }
}

std::string Node::stateToString(WindowNodeState state)
{
    switch (state)
    {
    case WindowNodeState::CanTransmit:
        return "Transmit";
    case WindowNodeState::CanListen:
        return " Listen";
    case WindowNodeState::CanCommunicate:
        return " Communicate";
    case WindowNodeState::CanSleep:
        return " Sleep";
    default:
        return "Unknown";
    }
}

//TODO: change std::chrono::milliseconds to int64_t for consistency
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



