#pragma once

#include <cstdint>
#include <vector>
#include <optional>

// Globally unique packet identifier
using GlobalPacketID = uint64_t;

// Time-series sample structure
template<typename T>
struct TimeSample {
    int64_t timestamp;  // ms
    T value;
};

// Latency statistics
struct LatencyStats {
    double mean = 0.0;
    double median = 0.0;
    double p95 = 0.0;      // 95th percentile
    double min = 0.0;
    double max = 0.0;
    size_t sampleCount = 0;
};

// Latency record for individual packets
struct LatencyRecord {
    GlobalPacketID packetId;
    int64_t latency_ms;
    int64_t originTimestamp;
    int64_t deliveryTimestamp;
};
