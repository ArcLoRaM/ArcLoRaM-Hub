#pragma once

#include "../../Network/Packets/IPacketController.hpp"
// #include "../NetworkVisualisationScreen.hpp"

#include "VisualiserManager.hpp"

class ProtocolPacketController : public IPacketController {


    //General Comment:
    //This class is creating the objects (devices, arrows....) and pass it to the manager.
    //Fine but would be better to give this task to the manager, makes more sense, encapsulate the logic
    //Todo: change this and from now on do that to the next screens


    private:
    //put state and manager as private members of the class, to avoid passing them as parameters
    void handleSystemPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleTickPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleStateNodePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handlePositionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleTransmitMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleReceiveMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleRoutingDecisionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleBroadcastMessagePacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleDropAnimationPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleRetransmissionPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);
    void handleStopSimulationPacket(sf::Packet& packet,ProtocolVisualisationState &state, VisualiserManager &manager);

public:
    void handlePacket(sf::Packet& packet, ProtocolVisualisationState& state, VisualiserManager& manager) override;
};
