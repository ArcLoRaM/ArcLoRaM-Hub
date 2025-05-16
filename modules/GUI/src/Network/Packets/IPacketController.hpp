// IPacketController.hpp
#pragma once

#include <SFML/Network.hpp>
#include "../../Screens/ProtocolVisualisationPackage/VisualiserManager.hpp"
#include "../../Screens/ProtocolVisualisationPackage/ProtocolVisualisationState.hpp"


class ProtocolVisualisationState; // Forward declaration to avoid circular dependency
class VisualiserManager; // Forward declaration to avoid circular dependency


//add this later when you will have more screens.
//template <typename State, typename Manager>  -> too complex and maybe overkilled

class IPacketController {
public:
    virtual ~IPacketController() = default;
    //there should be a State and Manager template parameter, mandatory when adding new screens.
    virtual void handlePacket(sf::Packet& packet, ProtocolVisualisationState& state, VisualiserManager& manager) = 0;
};
