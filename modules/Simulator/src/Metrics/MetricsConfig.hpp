#pragma once

#include "../Node/NodeEnums.hpp"
#include <map>

namespace MetricsConfig {

    // Energy model: Current draw per node state (in milliamperes)
    // Based on typical LoRa module specifications
    const std::map<NodeState, double> STATE_CURRENT_DRAW_MA = {
        {NodeState::Transmitting, 120.0},    // TX mode
        {NodeState::Listening, 15.0},        // RX mode
        {NodeState::Sleeping, 0.2},          // Sleep mode
        {NodeState::Communicating, 15.0}     // RX mode (listening while communicating)
    };

    // Time-series sampling interval (in ticks)
    // Default: sample every 1 tick
    constexpr unsigned int DEFAULT_SAMPLING_INTERVAL_TICKS = 1;

    // Battery voltage (for energy calculations if needed)
    constexpr double BATTERY_VOLTAGE_V = 3.3;

}  // namespace MetricsConfig
