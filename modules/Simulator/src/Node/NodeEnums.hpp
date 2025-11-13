#pragma once

#include <string>



// Enum representing possible states for nodes
enum class NodeState {
    Transmitting,
    Listening,
    Sleeping,
    Communicating
};
//Enum representing the scheduler proposed states for the nodes
enum class WindowNodeState{
    CanTransmit,
    CanListen,
    CanSleep,
    CanCommunicate //mixed state of CanTransmit and CanListen (the nodes listen and sometimes it transmits)
};

//Should be a copy of what's in the GUI
enum class ReceptionState {
    Interference,
    NotListening,
    Received
};

enum class NodeClass {
    C1 = 1,
    C2 = 2,
    C3 = 3,
    NotInitialized = -1
};

inline std::string stateToString(NodeState state) {
    switch (state) {
        case NodeState::Transmitting: return "Transmitting";
        case NodeState::Listening: return "Listening";
        case NodeState::Sleeping: return "Sleeping";
        case NodeState::Communicating: return "Communicating";
        default: return "UNKNOWN";
    }
}

inline std::string nodeClassToString(NodeClass nodeClass) {
    switch (nodeClass) {
        case NodeClass::C1: return "C1";
        case NodeClass::C2: return "C2";
        case NodeClass::C3: return "C3";
        case NodeClass::NotInitialized: return "NotInitialized";
        default: return "UNKNOWN";
    }
}




inline std::string toString(NodeState state) {

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



inline std::string windowStateToString(WindowNodeState state)
{
    switch (state)
    {
    case WindowNodeState::CanTransmit:
        return "CanTransmit";
    case WindowNodeState::CanListen:
        return "CanListen";
    case WindowNodeState::CanCommunicate:
        return "CanCommunicate";
    case WindowNodeState::CanSleep:
        return "CanSleep";
    default:
        return "Unknown";
    }
}

