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
#include <optional>
#include <future>
#include "NodeEnums.hpp"
#include "../TDMA/IScheduleBlueprint.hpp"

class PhyLayer; // Forward declaration of PhyLayer class to avoid circular dependency


class Node : public std::enable_shared_from_this<Node> {
public:

    Node(int id, Logger& logger,std::pair<int, int> coordinates, double batteryLevel=2.0);
    virtual ~Node() {
        
    }
    

    virtual std::string initMessage() const;//default message to be logged when the node starts


    //todo: do we need the bool ? why not just a void?
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

    void onTimeChange(WindowNodeState proposedState);
    virtual  int getClassId() const =0;
    virtual void handleCommunication()=0;//we separate state transition from the communication logic, this function is called after each state transition

    //todo: make other helpers to log events to the UI? TODO: move all of these functions to another class, one class per concern
    void adressedPacketTransmissionDisplay(uint16_t receiverId,bool isAck) const; // Display the transmission of a packet to a specific receiver
    void endAdressedPacketTransmissionDisplay(uint16_t receiverId) const; // Display the end of transmission of a packet to a specific receiver  
    void receptionStateDisplay(uint16_t senderId, ReceptionState state);
    void dropAnimationDisplay();

    //the isCommunicatingAck paremeter is used in the GUI to count the energy expenditure, as it consumes more energy to send a data packet than an ack packet (different TOA)
    //this will probably change in the future, TODO
    //TODO switch to an enum class, not string
    void nodeStateDisplay(std::string state, std::optional<bool> isCommunicatingAck);

//TDMA
    // Blueprint configuration
    void setScheduleBlueprint(std::shared_ptr<IScheduleBlueprint> bp);
    
    // Schedule the next state transition based on blueprint
    void scheduleNextTransition();
    
    // Allow nodes to schedule arbitrary future events (for dynamic strategies)
    void scheduleCustomTransition(int64_t absoluteTime, WindowNodeState state);
    
    // Query current simulation time
    int64_t getCurrentTime() const;

    protected:
    std::shared_ptr<IScheduleBlueprint> blueprint;

    void logEvent(const std::string& message) {
        logger.logEvent(nodeId, message);
    }
    double batteryLevel=3.0;
     NodeClass nodeClass= NodeClass::NotInitialized;
    std::pair<int, int> coordinates ={0,0};//in meters (x,y)
    int nodeId;
    Logger& logger;

    PhyLayer* phyLayer = nullptr; // Pointer to the PhyLayer instance, can be set later


    //methods
    //the node adds a message to the transmitting buffer and notifies the simulation manager
    void addMessageToTransmit(const std::vector<uint8_t>& message, int64_t airtimeMs);


    NodeState currentState; // the actual state of the node
    // Transition rules using functions for complex conditions: link a proposed state/current State with a callback that will check conditions and eventually change current state and perform actions
    std::map<std::pair<WindowNodeState, NodeState>,  std::function<bool()>> stateTransitions;
      

    NodeState convertWindowNodeStateToNodeState(WindowNodeState state);
    NodeState getCurrentState() const {
        
        return currentState;
    }
   void setCurrentState(NodeState newState) {
    
    currentState = newState;

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

  
    virtual bool canNodeReceiveMessage()=0;


    void initializeTransitionMap();
};
