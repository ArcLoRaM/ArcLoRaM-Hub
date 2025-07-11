#pragma once



#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include "../Connectivity/Logger/Logger.hpp"
#include <utility> // For std::pair
#include <functional> // For std::function
#include <optional>
#include <cmath> // For std::sqrt
#include <random>
#include <sstream>
#include <map>
#include <atomic>
#include "../Miscellaneous/PacketTool/PacketTool.hpp"
#include <optional>
#include <future>


// Enum representing possible states for nodes
enum class NodeState {
    Transmitting,
    Listening,
    Sleeping,
    Communicating
};

std::string toString(NodeState state);

//Enum representing the scheduler proposed states for the nodes
enum class WindowNodeState{
    
    CanTransmit,
    CanListen,
    CanSleep,
    CanCommunicate //mixed state of CanTransmit and CanListen
};


class PhyLayer; // Forward declaration of PhyLayer class to avoid circular dependency


class Node {
public:

    Node(int id, Logger& logger,std::pair<int, int> coordinates, double batteryLevel=2.0);
    virtual ~Node() {
        
    }

    //virtual void run()=0; //we shifted to an event driven model
    
  
     
    const std::vector<std::pair<int64_t, WindowNodeState>>& getActivationSchedule() const;

    virtual std::string initMessage() const;//default message to be logged when the node starts

    //used by simulation manager

    //todo: do we need the bool ? why not just a void?
    //add a message to the receiving buffer if no interference is detected, additionnal behaviour can be added in child classes
    virtual bool receiveMessage(const std::vector<uint8_t> message);

    //getters
    int getId() const { return nodeId; }
    int getXCoordinate() const {
        return coordinates.first;
    }
    int getYCoordinate() const {
        return coordinates.second;
    }


    void setPhyLayer(PhyLayer* phy){
        phyLayer = phy;
    }
    
     //add TDMA
     void addActivation( int64_t activationTime, WindowNodeState activationState);
             
    
    void onTimeChange(WindowNodeState proposedState);
     virtual  int getClassId() const =0;
    //for the moment, no parameters, later we need to add the time_ms
    virtual void handleCommunication()=0;//we separate state transition from the communication logic, this function is called after each state transition
    static std::string stateToString(NodeState state);
    static std::string stateToString(WindowNodeState state);
protected:

    void logEvent(const std::string& message) {
        logger.logEvent(nodeId, message);
    }
    double batteryLevel=3.0;

    std::pair<int, int> coordinates ={0,0};//in meters (x,y)
    int nodeId;
    bool running;
    Logger& logger;

    PhyLayer* phyLayer = nullptr; // Pointer to the PhyLayer instance, can be set later


    //methods
    //the node adds a message to the transmitting buffer and notifies the simulation manager
    void addMessageToTransmit(const std::vector<uint8_t>& message, int64_t airtimeMs);



    //---------------------------------------TDMA-------------------------------------
    std::vector<std::pair<int64_t, WindowNodeState>> activationSchedule; // the list of proposed node state (window State) by the scheduler at a given time
    NodeState currentState; // the actual state of the node
    // Transition rules using functions for complex conditions: link a proposed state/current State with a callback that will check conditions and eventually change current state and perform actions
    std::map<std::pair<WindowNodeState, NodeState>,  std::function<bool()>> stateTransitions;
      
    void setInitialState(NodeState initialState) {
        currentState = initialState;
    }
    NodeState convertWindowNodeStateToNodeState(WindowNodeState state);
    NodeState getCurrentState() {
        
        return currentState;
    }
   void setCurrentState(NodeState newState) {
    
    currentState = newState;

    //Todo: put the state switch here for the GUI
}



    //we need to define in child classes the state machine
    //convention for the name of the methods:
    //proposedWindowStateFromCurrentState: IdleFromTransmit

    virtual bool canTransmitFromListening()=0;
    virtual bool canTransmitFromSleeping()=0;
    virtual bool canTransmitFromTransmitting()=0;
    virtual bool canTransmitFromCommunicating()=0;

    virtual bool canListenFromTransmitting()=0;
    virtual bool canListenFromSleeping()=0;
    virtual bool canListenFromListening()=0;
    virtual bool canListenFromCommunicating()=0;

    virtual bool canSleepFromTransmitting()=0;
    virtual bool canSleepFromListening()=0;
    virtual bool canSleepFromSleeping()=0;
    virtual bool canSleepFromCommunicating()=0;

    virtual bool canCommunicateFromTransmitting()=0;
    virtual bool canCommunicateFromListening()=0;
    virtual bool canCommunicateFromSleeping()=0;
    virtual bool canCommunicateFromCommunicating()=0;

  
    void initializeTransitionMap();





    // Display methods
    //Todo: put the other display methods (broadcast, routing, init etc.) for every mode for every class !
    //Todo: the reception state should be a struct, not raw strings
    void adressedPacketTransmissionDisplay(uint16_t receiverId,bool isAck) const; // Display the transmission of a packet to a specific receiver
    // Visualiser (aka GUI) display
    void receptionStateDisplay(uint16_t senderId, std::string state);
    void dropAnimationDisplay();
    void nodeStateDisplay(std::string state, std::optional<bool> isCommunicatingAck);
};
