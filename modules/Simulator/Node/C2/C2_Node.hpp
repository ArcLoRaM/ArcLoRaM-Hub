#pragma once
#include "../Node.hpp"
#include "../../Setup/Common.hpp"
#include "../../Connectivity/TCP/packets.hpp"
#include "C2RccUplinkSlotManager.hpp"
// TODO:
// instead of having multiple variables, prepare struct or objects that will contain the information needed to pass logic between receive() and transmit()

class C2_Node : public Node
{

public:
#if COMMUNICATION_PERIOD == RRC_DOWNLINK || COMMUNICATION_PERIOD == RRC_BEACON

    C2_Node(int id, Logger &logger, std::pair<int, int> coordinates, std::condition_variable &dispatchCv, std::mutex &dispatchCvMutex, double batteryLevel = 0)
        : Node(id, logger, coordinates, dispatchCv, dispatchCvMutex, batteryLevel)
    {

        initializeTransitionMap();
        setInitialState(NodeState::Sleeping);
    };

    // constructor that simulates a beacon session already happened -> routes are established
    C2_Node(int id, Logger &logger, std::pair<int, int> coordinates, std::condition_variable &dispatchCv, std::mutex &dispatchCvMutex, double batteryLevel, int nextNodeId, int hopCount)
        : Node(id, logger, coordinates, dispatchCv, dispatchCvMutex, batteryLevel)
    {

#if TOPOLOGY == MESH_SELF_HEALING
        // I lack of time, so I will not provision the full initial state that should result from a beacon mode,
        // I will simply display the route but internally, it's as "if" nothing happened. The topology using this constructor is built in a way that we don't see it.
        // TODO: Implement this initial state: nextNodeIdInPath, basePathCost, hopCount....
        this->nextNodeIdInPath = nextNodeId;

        initializeTransitionMap();

#endif

        setInitialState(NodeState::Sleeping);
    };
#elif COMMUNICATION_PERIOD == RRC_UPLINK
    C2_Node(int id, Logger &logger, std::pair<int, int> coordinates,  uint16_t nextNodeIdInPath, uint8_t hopCount)
        : Node(id, logger, coordinates), infoFromBeaconPhase{nextNodeIdInPath, hopCount} 
    {

        initializeTransitionMap();
        setInitialState(NodeState::Sleeping);
        // //Display the routing if visualiser connected
        // if (common::visualiserConnected)
        // {
        //     displayRouting();
        // }

        // decide which slots among the  speci will actually be possible DATA and modulo slots actually used to transmit information
        slotManager.initializeRandomSlots(common::maxNodeSlots, common::totalNumberOfSlotsPerModuloNode);

        // Build the message string showing the selected slots
        std::string slotsLogMsg = "Node " + std::to_string(nodeId) + " selected transmission slots: [";
        const auto &slots = slotManager.getSlots();

        for (size_t i = 0; i < slots.size(); ++i)
        {
            slotsLogMsg += std::to_string(slots[i]);
            if (i != slots.size() - 1)
            {
                slotsLogMsg += ", ";
            }
        }
        slotsLogMsg += "]";

        // Log the message
        Log slotLog(slotsLogMsg, true);
        logger.logMessage(slotLog);

        //The node will use the allowed category of slots to transmit
        fixedSlotCategory = infoFromBeaconPhase.getHopCount() % 3;
        nbPayloadLeft = initialnbPaylaod;

    };

#else
#error "Unknown COMMUNICATION_PERIOD mode"
#endif
    int getClassId() const override
    {
        return 2;
    }

    std::string initMessage() const override;

protected:
    bool canTransmitFromListening();
    bool canTransmitFromSleeping();
    bool canTransmitFromTransmitting();
    bool canTransmitFromCommunicating();

    bool canListenFromTransmitting();
    bool canListenFromSleeping();
    bool canListenFromListening();
    bool canListenFromCommunicating();

    bool canSleepFromTransmitting();
    bool canSleepFromListening();
    bool canSleepFromSleeping();
    bool canSleepFromCommunicating();

    bool canCommunicateFromTransmitting();
    bool canCommunicateFromListening();
    bool canCommunicateFromSleeping();
    bool canCommunicateFromCommunicating();

    bool receiveMessage(const std::vector<uint8_t> message, std::chrono::milliseconds timeOnAir) override;

    void handleCommunication() ;
#if COMMUNICATION_PERIOD == RRC_DOWNLINK

    bool shouldSendBeacon = false;
    std::vector<int> beaconSlots;
    std::vector<uint16_t> globalIDPacketList; //
    bool canNodeReceiveMessage();
    bool isTransmittingWhileCommunicating = false;

    // this variable contains the id of the node that is the final receiver of the flooding packet circulating in the network
    // use to bridge the receive function and the transmit function
    std::optional<uint32_t> packetFinalReceiverId;

#elif COMMUNICATION_PERIOD == RRC_BEACON

    // This values should be a struct or an object, todo
    bool shouldSendBeacon = false;
    std::optional<uint8_t> hopCount;
    std::optional<uint32_t> lastTimeStampReceived;
    std::vector<int> beaconSlots;
    std::vector<uint16_t> globalIDPacketList; //
    std::optional<uint8_t> pathCost;          // the value that will be sent in the beacon packets
    std::optional<uint8_t> basePathCost;      // the value that serves as reference for the most optimal path, found in the beacon received
    std::optional<uint16_t> nextNodeIdInPath; // The Id of the node in the optimal route
    bool canNodeReceiveMessage();
    bool isTransmittingWhileCommunicating = false;

#elif COMMUNICATION_PERIOD == RRC_UPLINK



    // visualiser---------------------------------------------------------------------------------------
    void displayRouting(); // we cannot put this in the constructor as we need to wait for the visualiser to receive all the nodes
                           //it's quite specific to the C2_Node, so we put it here
    bool routingDisplayed = false;

    // Reception---------------------------------------------------------------------------------------
    bool canNodeReceiveMessage();
    void handleDataPacketReception(const std::vector<uint8_t> &message, uint16_t senderId, uint32_t packetId);
    void handleAckPacketReception( uint16_t senderId, uint32_t packetId);
    bool isTransmittingWhileCommunicating = false;

    // Transmission------------------------------------------------------------------------------------
    void buildAndTransmitDataPacket(std::vector<uint8_t> payload={});
    void buildAndTransmitAckPacket();
    unsigned int localIDPacketCounter = 0; //the local Id of the packet we send. Uniquely identify packets in a link.

    //Todo: Implement an architecture with buffers, below is a simplification that only considers packets unicity
    uint8_t nbPayloadLeft;        // the number of payload left to send(initial + forward packet)(represents the data that will be sent, in the simulation, every payload is the same (0xFF...FF))
    uint8_t initialnbPaylaod = 2; // initial number of payload


    // Slot Strategy ---------------------------------------------------------------------------------------
    bool isACKSlot = true;
    // std::vector<int> transmissionSlots; // the slots where the node WILL transmit (unless if no data to send, in that case nothing happens), it's computed at the beginning of the simulation
    C2RccUplinkSlotManager slotManager; // the slots where the node CAN transmit (the slots that are not used by other nodes)
    bool handleAckSlotPhase();
    bool handleDataSlotPhase();
    uint8_t currentDataSlotCategory = 0;
    uint8_t fixedSlotCategory;
    // Packet MAP: we need the packet Map to not forward already forwarded data packet (ack can be lost which leads to retransmission of the same Data packet)
    using SenderID = uint16_t;
    using PacketID = uint16_t;
    using PacketList = std::vector<PacketID>;                 
    using PacketMap = std::unordered_map<SenderID, PacketList>; 
    PacketMap packetsMap; 

    // Struct -------------------------------------------------------------------------------------------

    struct InformationFromBeaconPhase
    {
        //Contains informations provided during beacon phase that are necessary for Mesh operations.
        //Manual and static initialization of the values for now.
    private:
        std::optional<uint16_t> nextNodeIdInPath;
        std::optional<uint8_t> hopCount;

    public:


        // Constructor allowing initialization from parameters
        InformationFromBeaconPhase(std::optional<uint16_t> nodeId, std::optional<uint8_t> hops) 
        : nextNodeIdInPath{nodeId}, hopCount{hops} {}




        // Setter for nextNodeIdInPath
        void setNextNodeIdInPath(uint16_t nodeId) noexcept
        {
            nextNodeIdInPath = nodeId;
        }

        // Setter for hopCount
        void setHopCount(uint8_t count) noexcept
        {
            hopCount = count;
        }

        // Getter for nextNodeIdInPath with validation
        [[nodiscard]] uint16_t getNextNodeIdInPath() const
        {
            if (!nextNodeIdInPath.has_value())
            {
                throw std::runtime_error("Next node ID in path is not set");
            }
            return *nextNodeIdInPath;
        }

        // Getter for hopCount with validation
        [[nodiscard]] uint8_t getHopCount() const
        {
            if (!hopCount.has_value())
            {
                throw std::runtime_error("Hop count is not set");
            }
            return *hopCount;
        }

        uint8_t getModulatedHopCount() const 
        {   if (!hopCount.has_value())
            {
                throw std::runtime_error("Hop count is not set");
            }
            // Modulate the hop count to fit in the range of 0-2
            return *hopCount % 3 ;
        }

        // Checkers
        [[nodiscard]] bool hasNextNodeIdInPath() const noexcept
        {
            return nextNodeIdInPath.has_value();
        }

        [[nodiscard]] bool hasHopCount() const noexcept
        {
            return hopCount.has_value();
        }

        // Reset both to uninitialized state
        void reset() noexcept
        {
            nextNodeIdInPath.reset();
            hopCount.reset();
        }
    };

    struct RetransmissionCounterHelper
    {
        //  retransmission  is a metric in the visualiser)
    private:
        bool isExpectingAck{false};
        bool secondSleepWindow{true};

    public:
        // Setter for isExpectingAck with parameter
        void setIsExpectingAck(bool expecting) noexcept
        {
            isExpectingAck = expecting;
        }

        // Setter for secondSleepWindow that toggles its value
        void toggleSecondSleepWindow() noexcept
        {
            secondSleepWindow = !secondSleepWindow;
        }

        // Getter for isExpectingAck
        [[nodiscard]] bool getIsExpectingAck() const noexcept
        {
            return isExpectingAck;
        }

        // Getter for secondSleepWindow
        [[nodiscard]] bool getSecondSleepWindow() const noexcept
        {
            return secondSleepWindow;
        }
    };

    struct AckInformation
    {
    private:
        std::optional<uint16_t> lastSenderId;
        std::optional<uint16_t> lastLocalIDPacket;
        bool replyAck{false};

    public:
        void setNewAckInformation(uint16_t lastSenderId, uint16_t lastLocalIDPacket) noexcept
        {
            this->lastSenderId = lastSenderId;
            this->lastLocalIDPacket = lastLocalIDPacket;
            replyAck = true;
        }

        bool shouldReplyAck() const noexcept
        {
            return replyAck;
        }

        std::pair<uint16_t, uint16_t> getAndResetAckInformation()
        {
            if (!lastSenderId.has_value() || !lastLocalIDPacket.has_value())
            {
                throw std::runtime_error("AckInformation not set");
            }

            // Use std::exchange to get the values and reset to nullopt
            auto senderId = std::exchange(lastSenderId, std::nullopt);
            auto localID = std::exchange(lastLocalIDPacket, std::nullopt);

            replyAck = false;

            return {senderId.value(), localID.value()};
        }
    };

    AckInformation ackInformation;
    RetransmissionCounterHelper retransmissionCounterHelper;
    InformationFromBeaconPhase infoFromBeaconPhase;
    // End - Struct -------------------------------------------------------------------------------------------

#else
#error "Unknown COMMUNICATION_PERIOD mode"
#endif
};
