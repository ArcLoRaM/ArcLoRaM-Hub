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
    //Should not be set too low to avoid excessive data and overflooding the TCP channel
    //For now, only used for energy time-series sampling
    constexpr unsigned int DEFAULT_SAMPLING_INTERVAL_TICKS = 100;

    // Metrics export interval (in ticks)
    // How often to send metrics to GUI via TCP
    constexpr int METRICS_EXPORT_INTERVAL_TICKS = 300;

    // Battery voltage (for energy calculations if needed)
    constexpr double BATTERY_VOLTAGE_V = 3.3;

}  
