#include "TdmaScheduleBlueprint.hpp"
#include <sstream>
#include <iomanip>

TdmaScheduleBlueprint::TdmaScheduleBlueprint(int64_t masterCycleDurationMs)
    : masterCycleDurationMs(masterCycleDurationMs) {
}

void TdmaScheduleBlueprint::defineMasterCycle(const std::vector<ModeSegment>& segments) {
    masterCycleTimeline.clear();
    
    int64_t accumulatedTime = 0;
    for (const auto& seg : segments) {
        if (seg.durationMs <= 0) {
            throw std::invalid_argument("Mode segment duration must be positive");
        }
        
        ModeSegmentInternal internal;
        internal.mode = seg.mode;
        internal.startTimeInCycle = accumulatedTime;
        internal.endTimeInCycle = accumulatedTime + seg.durationMs;
        
        if (internal.endTimeInCycle > masterCycleDurationMs) {
            throw std::runtime_error("Mode segments exceed master cycle duration");
        }
        
        masterCycleTimeline.push_back(internal);
        accumulatedTime += seg.durationMs;
    }
    
    if (accumulatedTime != masterCycleDurationMs) {
        throw std::runtime_error("Mode segments must fill entire master cycle. Expected: " + 
                                 std::to_string(masterCycleDurationMs) + "ms, Got: " + 
                                 std::to_string(accumulatedTime) + "ms");
    }
}

void TdmaScheduleBlueprint::setPatternForMode(TdmaMode mode, NodeClass nodeClass, 
                                                  std::vector<PatternStep> pattern) {
    if (nodeClass == NodeClass::NotInitialized) {
        throw std::invalid_argument("Cannot set pattern for NotInitialized node class");
    }
    
    if (pattern.empty()) {
        throw std::invalid_argument("Pattern cannot be empty");
    }
    
    for (const auto& step : pattern) {
        if (step.durationMs <= 0) {
            throw std::invalid_argument("Pattern step duration must be positive");
        }
    }
    
    modePatterns[mode][nodeClass] = std::move(pattern);
}
int64_t TdmaScheduleBlueprint::getTimeWithinMasterCycle(int64_t absoluteTime) const {
    if (absoluteTime < 0) {
        throw std::invalid_argument("Absolute time cannot be negative");
    }
    return absoluteTime % masterCycleDurationMs;
}

const TdmaScheduleBlueprint::ModeSegmentInternal& 
TdmaScheduleBlueprint::getModeSegmentAt(int64_t timeInCycle) const {
    if (masterCycleTimeline.empty()) {
        throw std::runtime_error("Master cycle timeline not initialized");
    }
    
    for (const auto& segment : masterCycleTimeline) {
        if (timeInCycle >= segment.startTimeInCycle && timeInCycle < segment.endTimeInCycle) {
            return segment;
        }
    }
    
    throw std::runtime_error("No mode segment found for time: " + std::to_string(timeInCycle) + 
                             "ms within cycle");
}

int64_t TdmaScheduleBlueprint::getTimeWithinModeSegment(int64_t absoluteTime) const {
    int64_t timeInCycle = getTimeWithinMasterCycle(absoluteTime);
    const auto& segment = getModeSegmentAt(timeInCycle);
    return timeInCycle - segment.startTimeInCycle;
}

const std::vector<PatternStep>* 
TdmaScheduleBlueprint::getPatternOrNull(TdmaMode mode, NodeClass nodeClass) const {
    auto modeIt = modePatterns.find(mode);
    if (modeIt == modePatterns.end()) {
        return nullptr;
    }
    
    auto classIt = modeIt->second.find(nodeClass);
    if (classIt == modeIt->second.end()) {
        return nullptr;
    }
    
    return &classIt->second;
}
int64_t TdmaScheduleBlueprint::calculatePatternDuration(const std::vector<PatternStep>& pattern) {
    int64_t total = 0;
    for (const auto& step : pattern) {
        total += step.durationMs;
    }
    return total;
}

std::string TdmaScheduleBlueprint::modeToString(TdmaMode mode) {
    switch (mode) {
        case TdmaMode::RRC_UPLINK:    return "RRC_UPLINK";
        case TdmaMode::RRC_DOWNLINK:  return "RRC_DOWNLINK";
        case TdmaMode::RRC_BEACON:    return "RRC_BEACON";
        case TdmaMode::ENC_UPLINK:    return "ENC_UPLINK";
        case TdmaMode::ENC_DOWNLINK:  return "ENC_DOWNLINK";
        case TdmaMode::ENC_BEACON:    return "ENC_BEACON";
        default:                      return "UNKNOWN";
    }
}

TdmaMode TdmaScheduleBlueprint::getCurrentMode(int64_t currentTime) const {
    int64_t timeInCycle = getTimeWithinMasterCycle(currentTime);
    return getModeSegmentAt(timeInCycle).mode;
}

std::optional<int64_t> TdmaScheduleBlueprint::getNextModeTransitionTime(int64_t currentTime) const {
    if (masterCycleTimeline.empty()) {
        return std::nullopt;
    }
    
    int64_t timeInCycle = getTimeWithinMasterCycle(currentTime);
    int64_t cycleNumber = currentTime / masterCycleDurationMs;
    
    // Find the current segment
    const auto* currentSegment = &getModeSegmentAt(timeInCycle);
    
    // Next transition is at the end of current segment
    int64_t nextTransitionInCycle = currentSegment->endTimeInCycle;
    
    // If we're at the last segment, next transition is at cycle wrap (start of cycle 0)
    if (nextTransitionInCycle >= masterCycleDurationMs) {
        return (cycleNumber + 1) * masterCycleDurationMs;
    }
    
    return cycleNumber * masterCycleDurationMs + nextTransitionInCycle;
}

std::optional<WindowNodeState> 
TdmaScheduleBlueprint::tryGetStateAt(int64_t currentTime, NodeClass nodeClass) const {
    if (nodeClass == NodeClass::NotInitialized) {
        throw std::invalid_argument("Cannot query state for NotInitialized node class");
    }
    
    TdmaMode currentMode = getCurrentMode(currentTime);
    const auto* pattern = getPatternOrNull(currentMode, nodeClass);
    
    if (!pattern) {
        return std::nullopt; // No pattern defined - free scheduling mode
    }
    
    int64_t timeInSegment = getTimeWithinModeSegment(currentTime);
    int64_t patternDuration = calculatePatternDuration(*pattern);
    int64_t timeInPattern = timeInSegment % patternDuration;
    
    int64_t accumulatedTime = 0;
    for (const auto& step : *pattern) {
        if (timeInPattern < accumulatedTime + step.durationMs) {
            return step.state;
        }
        accumulatedTime += step.durationMs;
    }
    
    throw std::runtime_error("Pattern state calculation failed - should never reach here");
}
std::optional<int64_t> 
TdmaScheduleBlueprint::tryGetNextTransitionTime(int64_t currentTime, NodeClass nodeClass) const {
    if (nodeClass == NodeClass::NotInitialized) {
        throw std::invalid_argument("Cannot query transition time for NotInitialized node class");
    }
    
    TdmaMode currentMode = getCurrentMode(currentTime);
    const auto* pattern = getPatternOrNull(currentMode, nodeClass);
    
    if (!pattern) {
        return std::nullopt; // No pattern defined - free scheduling mode
    }
    
    int64_t timeInCycle = getTimeWithinMasterCycle(currentTime);
    const auto& currentSegment = getModeSegmentAt(timeInCycle);
    int64_t timeInSegment = timeInCycle - currentSegment.startTimeInCycle;
    
    int64_t patternDuration = calculatePatternDuration(*pattern);
    int64_t timeInPattern = timeInSegment % patternDuration;
    
    // Find next step boundary within pattern
    int64_t accumulatedTime = 0;
    for (const auto& step : *pattern) {
        accumulatedTime += step.durationMs;
        if (timeInPattern < accumulatedTime) {
            // Calculate next transition time within segment
            int64_t nextTransitionInSegment = timeInSegment - timeInPattern + accumulatedTime;
            
            // Check if this exceeds current mode segment
            int64_t segmentDuration = currentSegment.endTimeInCycle - currentSegment.startTimeInCycle;
            if (nextTransitionInSegment >= segmentDuration) {
                // Mode change happens first - return mode transition time
                return getNextModeTransitionTime(currentTime);
            }
            
            // Pattern transition happens first
            int64_t cycleNumber = currentTime / masterCycleDurationMs;
            int64_t nextTransitionInCycle = currentSegment.startTimeInCycle + nextTransitionInSegment;
            return cycleNumber * masterCycleDurationMs + nextTransitionInCycle;
        }
    }
    
    throw std::runtime_error("Next transition calculation failed - should never reach here");
}
bool TdmaScheduleBlueprint::hasPatternForMode(TdmaMode mode, NodeClass nodeClass) const {
    if (nodeClass == NodeClass::NotInitialized) {
        return false;
    }
    return getPatternOrNull(mode, nodeClass) != nullptr;
}

std::string TdmaScheduleBlueprint::getScheduleDescription() const {
    std::ostringstream oss;
    
    // Master cycle info
    oss << "Master Cycle: " << masterCycleDurationMs << "ms (" 
        << (masterCycleDurationMs / 3600000.0) << "h)\n";
    
    // Mode timeline
    oss << "Mode Timeline:\n";
    if (masterCycleTimeline.empty()) {
        oss << "  [Not initialized]\n";
    } else {
        for (const auto& segment : masterCycleTimeline) {
            oss << "  [" << segment.startTimeInCycle << "-" << segment.endTimeInCycle 
                << "ms] " << modeToString(segment.mode) << "\n";
        }
    }
    
    // Patterns
    oss << "Patterns:\n";
    if (modePatterns.empty()) {
        oss << "  [None defined]\n";
    } else {
        for (const auto& [mode, classMap] : modePatterns) {
            for (const auto& [classId, pattern] : classMap) {
            oss << "  " << modeToString(mode) << " (" << nodeClassToString(classId) << "): ";
                
                bool first = true;
                for (const auto& step : pattern) {
                    if (!first) oss << " -> ";
                    oss << windowStateToString(step.state) << " " << step.durationMs << "ms";
                    first = false;
                }
                
                int64_t totalDuration = calculatePatternDuration(pattern);
                oss << " (" << totalDuration << "ms cycle)\n";
            }
        }
    }
    
    return oss.str();
}