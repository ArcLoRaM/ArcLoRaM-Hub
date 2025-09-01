#pragma once

#include "../../ModeHandler.hpp"
#include "../C3_Node.hpp"

class C3_RRC_UplinkHandler : public ModeHandler<C3_Node>
{
public:

    bool receiveMessage(C3_Node &node, const std::vector<uint8_t> &message) override;

    bool canNodeReceiveMessage(const C3_Node &node) const override;

    bool canTransmitFromListening(C3_Node &node) override;
    bool canTransmitFromSleeping(C3_Node &node) override;
    bool canTransmitFromTransmitting(C3_Node &node) override;
    bool canTransmitFromCommunicating(C3_Node &node) override;

    bool canListenFromTransmitting(C3_Node &node) override;
    bool canListenFromSleeping(C3_Node &node) override;
    bool canListenFromListening(C3_Node &node) override;
    bool canListenFromCommunicating(C3_Node &node) override;

    bool canSleepFromTransmitting(C3_Node &node) override;
    bool canSleepFromListening(C3_Node &node) override;
    bool canSleepFromSleeping(C3_Node &node) override;
    bool canSleepFromCommunicating(C3_Node &node) override;

    bool canCommunicateFromTransmitting(C3_Node &node) override;
    bool canCommunicateFromListening(C3_Node &node) override;
    bool canCommunicateFromSleeping(C3_Node &node) override;
    bool canCommunicateFromCommunicating(C3_Node &node) override;

    void handleCommunication(C3_Node &node) override;

private:
    void buildAndTransmitAckPacket(C3_Node &node);
};
