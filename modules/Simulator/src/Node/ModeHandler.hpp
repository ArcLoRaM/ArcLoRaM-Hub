#pragma once

#include <vector>
#include <cstdint>

/// Generic interface for implementing per-mode, per-node behavior.
/// Specialized for each node type: ModeHandler<C1_Node>, ModeHandler<C2_Node>, etc.
/// Used by the engine through Node’s virtual functions.
//polymorphic behavior
template <typename NodeType>
class ModeHandler {
public:
    virtual ~ModeHandler() = default;

    virtual bool receiveMessage(NodeType& node, const std::vector<uint8_t>& message) = 0;

    virtual bool canNodeReceiveMessage(const NodeType& node) const = 0;

    virtual bool canTransmitFromListening( NodeType& node)=0;
    virtual bool canTransmitFromSleeping(NodeType& node)=0;
    virtual bool canTransmitFromTransmitting(NodeType& node)=0;
    virtual bool canTransmitFromCommunicating(NodeType& node)=0;

    virtual bool canListenFromTransmitting(NodeType& node)=0;
    virtual bool canListenFromSleeping(NodeType& node)=0;
    virtual bool canListenFromListening(NodeType& node)=0;
    virtual bool canListenFromCommunicating(NodeType& node)=0;

    virtual bool canSleepFromTransmitting(NodeType& node)=0;
    virtual bool canSleepFromListening(NodeType& node)=0;
    virtual bool canSleepFromSleeping(NodeType& node)=0;
    virtual bool canSleepFromCommunicating(NodeType& node)=0;

    virtual bool canCommunicateFromTransmitting(NodeType& node)=0;
    virtual bool canCommunicateFromListening(NodeType& node)=0;
    virtual bool canCommunicateFromSleeping(NodeType& node)=0;
    virtual bool canCommunicateFromCommunicating(NodeType& node)=0;


    virtual void handleCommunication(NodeType& node) = 0;


};
