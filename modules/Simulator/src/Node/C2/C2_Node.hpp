#pragma once
#include "../Node.hpp"
#include "../../Setup/Common.hpp"
#include "../../Setup/PacketBuffer.hpp"
#include "../../Connectivity/TCP/Packets/Packets.hpp"
#include "../ModeHandler.hpp"
#include "../../Metrics/MetricsTypes.hpp"
#include <queue>

//RRC_Uplink
class C2_RRC_UplinkHandler; // forward declaration
class C2_RRC_UplinkSlotManager;
 struct InformationFromBeaconPhase;
 struct RetransmissionCounterHelper;
 struct AckInformation;

//RRC_Downlink


//RRC_Beacon


class C2_Node : public Node
{
    friend class C2_RRC_UplinkHandler;

public:
//  RRC_DOWNLINK ||  RRC_BEACON
C2_Node(int id, Logger& logger, std::pair<int, int> coordinates);

    ~C2_Node();


//  RRC_UPLINK
C2_Node(int id, Logger &logger, std::pair<int, int> coordinates,  uint16_t nextNodeIdInPath, uint8_t hopCount);
   
    int getClassId() const override
    {
        return static_cast<int>(nodeClass);
    }

    std::string initMessage() const override;

    
    template<typename HandlerType>
    void registerModeHandler(TdmaMode mode, std::unique_ptr<HandlerType> handler) {
        static_assert(std::is_base_of_v<ModeHandler<C2_Node>, HandlerType>, 
                    "HandlerType must derive from ModeHandler<C2_Node>");
        modeHandlers[mode] = std::move(handler);
        logEvent("Registered handler for mode: " + std::to_string(static_cast<int>(mode)));
    }
protected:
    // Map of mode -> handler
    std::map<TdmaMode, std::unique_ptr<ModeHandler<C2_Node>>> modeHandlers;
    
    // Get the current active handler based on blueprint's current mode
    ModeHandler<C2_Node>* getCurrentHandler();   

    bool canTransmitFromListening()override;
    bool canTransmitFromSleeping()override;
    bool canTransmitFromTransmitting()override;
    bool canTransmitFromCommunicating()override;

    bool canListenFromTransmitting()override;
    bool canListenFromSleeping()override;
    bool canListenFromListening()override;
    bool canListenFromCommunicating()override;

    bool canSleepFromTransmitting()override;
    bool canSleepFromListening()override;
    bool canSleepFromSleeping()override;
    bool canSleepFromCommunicating()override;

    bool canCommunicateFromTransmitting()override;
    bool canCommunicateFromListening()override;
    bool canCommunicateFromSleeping()override;
    bool canCommunicateFromCommunicating()override;

    bool receiveMessage(const std::vector<uint8_t> message) override;

    void handleCommunication()override;
    bool canNodeReceiveMessage()override;


//----------------------------------------RRC_DOWNLINK//----------------------------------------

    bool RRC_Downlink_shouldSendDownlink = false;
    std::vector<int> RRC_Downlink_slots;
    std::vector<uint16_t> RRC_Downlink_globalIDPacketList; //
    bool RRC_Downlink_isTransmittingWhileCommunicating = false;

    // this variable contains the id of the node that is the final receiver of the flooding packet circulating in the network
    // use to bridge the receive function and the transmit function
    std::optional<uint32_t> RRC_Downlink_packetFinalReceiverId;

////----------------------------------------RRC_BEACON //----------------------------------------

    // This values should be a struct or an object, todo and dont have uint8_t but rather an using as..
    bool RRC_BEACON_shouldSendBeacon = false;
    std::optional<uint8_t> RRC_BEACON_hopCount;
    std::optional<uint32_t> RRC_BEACON_lastTimeStampReceived;
    std::vector<int> RRC_BEACON_slots;
    std::vector<uint16_t> RRC_BEACON_globalIDPacketList; //
    std::optional<uint8_t> RRC_BEACON_pathCost;          // the value that will be sent in the beacon packets
    std::optional<uint8_t> RRC_BEACON_basePathcanCost;      // the value that serves as reference for the most optimal path, found in the beacon received
    std::optional<uint16_t> RRC_BEACON_nextNodeIdInPath; // The Id of the node in the optimal route
    bool RRC_BEACON_isTransmittingWhileCommunicating = false;




    //------------------------------------------------ RRC UPLINK STATE ------------------------------------------------



    // visualiser---------------------------------------------------------------------------------------
    void RRC_UPLINK_displayRouting(); // we cannot put this in the constructor as we need to wait for the visualiser to receive all the nodes
                           //it's quite specific to the C2_Node, so we put it here
    bool RRC_UPLINK_routingDisplayed = false;

    // Reception---------------------------------------------------------------------------------------

     bool RRC_UPLINK_isTransmittingWhileCommunicating = false;

    // Transmission------------------------------------------------------------------------------------

    unsigned int RRC_UPLINK_localIDPacketCounter = 0; //the local Id of the packet we send. Uniquely identify packets in a link.

    // Packet queue: unified FIFO queue for both originated and forwarded packets
    packet_buffer::PacketQueue RRC_UPLINK_packetQueue;
    uint8_t RRC_UPLINK_initialnbPayload = 3; // initial number of payload

//There are states for the slot strategy (so should be a separate class renamed depending on the strategy used)(such as SimonV1..) and state inherent to node mode (like size queue...)
    // Slot Strategy ---------------------------------------------------------------------------------------
    //todo: could make a proper struct for that
    bool RRC_UPLINK_isACKSlot = true;
    uint8_t RRC_UPLINK_currentDataSlotCategory = 0;
    uint8_t RRC_UPLINK_fixedSlotCategory;

    // Packet MAP: we need the packet Map to not forward already forwarded data packet (ack can be lost which leads to retransmission of the same Data packet)
    using PacketID = uint16_t; //do this for every packets / custom state node
    using SenderID = uint16_t;
    using PacketList = std::vector<PacketID>;
    using PacketMap = std::unordered_map<SenderID, PacketList>;
    PacketMap RRC_UPLINK_packetsMap;

    std::unique_ptr<C2_RRC_UplinkSlotManager> RRC_UPLINK_slotManager; // the slots where the node CAN transmit (the slots that are not used by other nodes)

    std::unique_ptr<AckInformation> RRC_UPLINK_ackInformation;
    std::unique_ptr<RetransmissionCounterHelper> RRC_UPLINK_retransmissionCounterHelper;
    std::unique_ptr<InformationFromBeaconPhase> RRC_UPLINK_infoFromBeaconPhase;



};
