#pragma once
#include "../IScheduleBlueprint.hpp"
#include <map>
#include <vector>
#include <stdexcept>


class TdmaScheduleBlueprint : public IScheduleBlueprint {
public:
    // Constructor with configurable master cycle duration (default 24 hours)
    explicit TdmaScheduleBlueprint(int64_t masterCycleDurationMs = 24 * 3600 * 1000);
    
    // Configuration API - used during setup to define the blueprint
    
    struct ModeSegment {
        TdmaMode mode;
        int64_t durationMs;
    };
    
    // Define the high-level mode timeline (must sum to masterCycleDurationMs)
    void defineMasterCycle(const std::vector<ModeSegment>& segments);
    
    // Define a pattern for a specific mode and node class
    // If not called for a mode/class combination, that combination has no pattern (free scheduling)
    void setPatternForMode(TdmaMode mode, NodeClass nodeClass, std::vector<PatternStep> pattern);
    
    // IScheduleBlueprint implementation
    TdmaMode getCurrentMode(int64_t currentTime) const override;
    std::optional<int64_t> getNextModeTransitionTime(int64_t currentTime) const override;
    std::optional<WindowNodeState> tryGetStateAt(int64_t currentTime, NodeClass nodeClass) const override;
    std::optional<int64_t> tryGetNextTransitionTime(int64_t currentTime, NodeClass nodeClass) const override;
    bool hasPatternForMode(TdmaMode mode, NodeClass nodeClass) const override;
    std::string getScheduleDescription() const override;

private:
    // Master cycle configuration
    int64_t masterCycleDurationMs;
    
    // Internal representation of mode timeline
    struct ModeSegmentInternal {
        TdmaMode mode;
        int64_t startTimeInCycle;  // Relative to cycle start (0 to masterCycleDurationMs)
        int64_t endTimeInCycle;    // Exclusive end
    };
    std::vector<ModeSegmentInternal> masterCycleTimeline;
    
    // Patterns: modePatterns[mode][nodeClass] = pattern
    // If entry doesn't exist, that mode/class has no pattern (free scheduling)
    std::map<TdmaMode, std::map<NodeClass, std::vector<PatternStep>>> modePatterns;
    
    // Helper functions
    
    // Normalize absolute simulation time to position within 24-hour cycle
    int64_t getTimeWithinMasterCycle(int64_t absoluteTime) const;
    
    // Find which mode segment contains the given time (within cycle)
    const ModeSegmentInternal& getModeSegmentAt(int64_t timeInCycle) const;
    
    // Get time elapsed since the start of the current mode segment
    int64_t getTimeWithinModeSegment(int64_t absoluteTime) const;
    
    // Get the pattern for a mode/class, returns nullptr if none exists
    const std::vector<PatternStep>* getPatternOrNull(TdmaMode mode, NodeClass nodeClass) const;
    
    // Calculate total duration of a pattern
    static int64_t calculatePatternDuration(const std::vector<PatternStep>& pattern);
    
    // Convert TdmaMode enum to string for logging
    static std::string modeToString(TdmaMode mode);
};