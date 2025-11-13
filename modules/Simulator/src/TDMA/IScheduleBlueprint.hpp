#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <string>
#include "../Node/NodeEnums.hpp"

/*
For future proofing and flexibility, we define an interface for TDMA schedule blueprints.
This allows to implement other protocols (e.g. CSMA/CA) or custom scheduling schemes in the future.
*/


enum class TdmaMode {
    RRC_UPLINK,
    RRC_DOWNLINK,
    RRC_BEACON,
    ENC_UPLINK,
    ENC_DOWNLINK,
    ENC_BEACON
};

enum class WindowNodeState; // Already defined in Node.hpp

struct PatternStep {
    WindowNodeState state;
    int64_t durationMs;
};


//TODO: is the interface really necessar?
class IScheduleBlueprint {
public:
    virtual ~IScheduleBlueprint() = default;
    
    // Core query API for nodes
    
    // Returns the current mode at the given time
    virtual TdmaMode getCurrentMode(int64_t currentTime) const = 0;
    
    // Returns the absolute time when the next mode transition occurs
    // Returns std::nullopt if no mode transition exists (infinite current mode)
    virtual std::optional<int64_t> getNextModeTransitionTime(int64_t currentTime) const = 0;
    
    // Attempts to get the state for a node at the given time
    // Returns std::nullopt if the current mode has no pattern (free scheduling mode)
virtual std::optional<WindowNodeState> tryGetStateAt(int64_t currentTime, NodeClass nodeClass) const = 0;
    
    // Attempts to get the next state transition time for a node
    // Returns std::nullopt if the current mode has no pattern (free scheduling mode)
    // If a mode transition happens before the pattern transition, returns the mode transition time
    virtual std::optional<int64_t> tryGetNextTransitionTime(int64_t currentTime, NodeClass nodeClass) const = 0;
    
    // Check if a specific mode has a defined pattern for a node class
    virtual bool hasPatternForMode(TdmaMode mode, NodeClass nodeClass) const = 0;
    
    // Logging/debugging
    // Returns a formatted string describing the complete TDMA schedule
    // Format example:
    // "Master Cycle: 86400000ms (24h)
    //  Mode Timeline:
    //    [0-300000ms] RRC_UPLINK
    //    [300000-420000ms] RRC_BEACON
    //  Patterns:
    //    RRC_UPLINK (Class 2): Sleep 100ms -> Listen 20ms -> Sleep 10ms -> Transmit 5ms (135ms cycle)
    //    RRC_BEACON (Class 3): [Free scheduling - no pattern]"
    virtual std::string getScheduleDescription() const = 0;
};