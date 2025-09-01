#pragma once

#include "../../ModeHandler.hpp"
#include "../C2_Node.hpp"

class C2_RRC_UplinkHandler : public ModeHandler<C2_Node> {
public:
    // Engine hooks
    bool receiveMessage(C2_Node& node, const std::vector<uint8_t>& message) override;

    // Listening
    bool canNodeReceiveMessage(const C2_Node& node) const override;

    bool canTransmitFromListening(C2_Node& node) override;
    bool canTransmitFromSleeping(C2_Node& node) override;
    bool canTransmitFromTransmitting(C2_Node& node) override;
    bool canTransmitFromCommunicating(C2_Node& node) override;

    bool canListenFromTransmitting(C2_Node& node) override;
    bool canListenFromSleeping(C2_Node& node) override;
    bool canListenFromListening(C2_Node& node) override;
    bool canListenFromCommunicating(C2_Node& node) override;

    bool canSleepFromTransmitting(C2_Node& node) override;
    bool canSleepFromListening(C2_Node& node) override;
    bool canSleepFromSleeping(C2_Node& node) override;
    bool canSleepFromCommunicating(C2_Node& node) override;

    bool canCommunicateFromTransmitting(C2_Node& node) override;
    bool canCommunicateFromListening(C2_Node& node) override;
    bool canCommunicateFromSleeping(C2_Node& node) override;
    bool canCommunicateFromCommunicating(C2_Node& node) override;

    void handleCommunication(C2_Node& node) override;

    private:
    void buildAndTransmitDataPacket(C2_Node& node,std::vector<uint8_t> payload) ;
    void buildAndTransmitAckPacket(C2_Node& node) ;
    void handleDataPacketReception(C2_Node &node,const std::vector<uint8_t> &message, uint16_t senderId, uint32_t packetId);
    void handleAckPacketReception(C2_Node &node,uint16_t senderId, uint32_t packetId);
};


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