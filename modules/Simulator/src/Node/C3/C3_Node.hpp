#pragma once

#include "../../Setup/Common.hpp"
#include "../Node.hpp"
#include "../../Connectivity/TCP/Packets/Packets.hpp"
#include <unordered_set>
#include "../ModeHandler.hpp"
#include "../../Setup/Common.hpp"

using namespace common;

class C3_RRC_UplinkHandler;

class C3_Node : public Node
{

    friend class C3_RRC_UplinkHandler;

public:
    C3_Node(int id, Logger &logger, std::pair<int, int> coordinates);

    int getClassId() const override
    {
        return 3;
    }
    std::string initMessage() const override;

    void setHandler(std::unique_ptr<ModeHandler<C3_Node>> h)
    {
        modeHandler = std::move(h);
    }

    // todo: make this function const?
    bool canNodeReceiveMessage() override;

protected:
    std::unique_ptr<ModeHandler<C3_Node>> modeHandler;
    bool canTransmitFromListening() override;
    bool canTransmitFromSleeping() override;
    bool canTransmitFromTransmitting() override;
    bool canTransmitFromCommunicating() override;

    bool canListenFromTransmitting() override;
    bool canListenFromSleeping() override;
    bool canListenFromListening() override;
    bool canListenFromCommunicating() override;

    bool canSleepFromTransmitting() override;
    bool canSleepFromListening() override;
    bool canSleepFromSleeping() override;
    bool canSleepFromCommunicating() override;

    bool canCommunicateFromTransmitting() override;
    bool canCommunicateFromListening() override;
    bool canCommunicateFromSleeping() override;
    bool canCommunicateFromCommunicating() override;

    void handleCommunication() override;
    bool receiveMessage(const std::vector<uint8_t> message) override;

    // RRC_BEACON

    std::vector<int> RRC_BEACON_slots; // ex: {0,  3, 4, 9} -> beacon to send now, in three slots, in four slots, in nine slots
    bool RRC_BEACON_shouldSendBeacon = true;

    // RRC_DOWNLINK
    // todo: rename these names (not beacons)
    std::vector<int> RRC_DOWNLINK_slots;
    bool RRC_DOWNLINK_shouldSendDownlink = true;

    // RRC_UPLINK

    bool RRC_UPLINK_shouldReplyACK = false;
    uint16_t RRC_UPLINK_lastSenderId;
    uint16_t RRC_UPLINK_lastLocalIDPacket;

    std::unordered_map<int, std::unordered_set<int>> RRC_UPLINK_receivedPacketsId; // Map to store received packets by sender ID
};
