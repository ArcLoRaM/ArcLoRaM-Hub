#include "SimonV1Factory.hpp"
#include "../../../Node/C3/C3_Node.hpp"
#include "../../../Node/C2/C2_Node.hpp"
#include "../../../Node/C1/C1_Node.hpp"
#include "../../../Setup/Common.hpp"

#include "../../../Node/C3/RRC_Uplink/C3_RRC_UplinkHandler.hpp"
#include "../../../Node/C2/RRC_Uplink/C2_RRC_UplinkHandler.hpp"


#include <stdexcept>

SimonV1Factory::SimonV1Factory(Logger& logger)
    : logger(logger) {
    
    // Create and configure the blueprint for SimonV1 scenario
    blueprint = std::make_shared<TdmaScheduleBlueprint>(24 * 3600 * 1000); // 24-hour cycle TODO: should be configurable (max Simulation time?)
    configureBlueprint();
}

void SimonV1Factory::configureBlueprint() {
    // Define the master cycle mode timeline for SimonV1
    // Example: Simple repeating pattern of RRC_UPLINK mode

    std::vector<TdmaScheduleBlueprint::ModeSegment> modeTimeline = {
        {TdmaMode::RRC_UPLINK, 24 * 3600 * 1000} // Entire 24h in RRC_UPLINK for now TODO: should be configurable
    };

    blueprint->defineMasterCycle(modeTimeline);
    
    // Define patterns for C3 nodes in RRC_UPLINK mode
    // Based on your original RrcUplinkNodeFactory timing
    using namespace common::rrc_uplink;
    
    std::vector<PatternStep> c3Pattern = {
        {WindowNodeState::CanSleep, durationSleepWindowMain_ms},
        {WindowNodeState::CanListen, durationDataWindow_ms},
        {WindowNodeState::CanSleep, durationSleepWindowSecondary_ms},
        {WindowNodeState::CanTransmit, durationACKWindow_ms}
    };
    blueprint->setPatternForMode(TdmaMode::RRC_UPLINK, NodeClass::C3, c3Pattern);
    
    // Define patterns for C2 nodes in RRC_UPLINK mode
    std::vector<PatternStep> c2Pattern = {
        {WindowNodeState::CanSleep, durationSleepWindowMain_ms},
        {WindowNodeState::CanCommunicate, durationDataWindow_ms},
        {WindowNodeState::CanSleep, durationSleepWindowSecondary_ms},
        {WindowNodeState::CanCommunicate, durationACKWindow_ms}
    };
    blueprint->setPatternForMode(TdmaMode::RRC_UPLINK, NodeClass::C2, c2Pattern);
    
    // C1 nodes: no pattern defined for RRC modes (they sleep)
    // Don't call setPatternForMode for C1 - they'll be in free scheduling mode (which means sleep)
    
    logger.logSystem("SimonV1 blueprint configured:");
    logger.logSystem(blueprint->getScheduleDescription());
}

std::shared_ptr<C3_Node> SimonV1Factory::createC3Node(int id, std::pair<int, int> coordinates) {
    auto node = std::make_shared<C3_Node>(id, logger, coordinates);
    
    // Attach the blueprint to the node
    node->setScheduleBlueprint(blueprint);

    // Register mode handlers - need explicit cast to base type
    node->registerModeHandler(TdmaMode::RRC_UPLINK, std::make_unique<C3_RRC_UplinkHandler>());

    // When you implement other modes:
    // node->registerModeHandler(TdmaMode::RRC_BEACON, std::make_unique<C3_RRC_BeaconHandler>());
    // node->registerModeHandler(TdmaMode::RRC_DOWNLINK, std::make_unique<C3_RRC_DownlinkHandler>());
    // etc.    
    return node;
}

std::shared_ptr<C2_Node> SimonV1Factory::createC2Node(int id, std::pair<int, int> coordinates, 
                                                       std::optional<int> nextHop, 
                                                       std::optional<int> hopCount) {
    std::shared_ptr<C2_Node> node=nullptr;
    if(!nextHop.has_value() || !hopCount.has_value()) {
        node= std::make_shared<C2_Node>(id, logger, coordinates);   
    }
    else node = std::make_shared<C2_Node>(id, logger, coordinates, nextHop.value(), 
                                          hopCount.value());

    // Register mode handlers - template automatically handles the conversion
    node->registerModeHandler(TdmaMode::RRC_UPLINK, std::make_unique<C2_RRC_UplinkHandler>());
    // Attach the blueprint to the node
    node->setScheduleBlueprint(blueprint);
    
    return node;
}

std::shared_ptr<C1_Node> SimonV1Factory::createC1Node(int id, std::pair<int, int> coordinates) {
    // C1 nodes not supported in SimonV1 scenario (RRC-focused)
    throw std::logic_error("C1 nodes are not supported in SimonV1 scenario.");
}