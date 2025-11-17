#pragma once

#include "MetricsTypes.hpp"
#include "MetricsConfig.hpp"
#include "../Node/NodeEnums.hpp"
#include <map>
#include <vector>
#include <cstdint>
#include "../Connectivity/Logger/Logger.hpp"

class NodeMetrics {
public:
    NodeMetrics(uint16_t nodeId, NodeClass nodeClass, Logger& logger);
    ~NodeMetrics() = default;

    // Get node ID
    uint16_t getNodeId() const { return nodeId; }
    NodeClass getNodeClass() const { return nodeClass; }

    // === Energy Tracking ===

    // Called when node transitions to a new state
    void onStateChange(NodeState newState, int64_t timestamp);

    // Sample current energy for time-series (called periodically by Clock)
    void sampleEnergyTimeSeries(int64_t timestamp);

    // Get cumulative energy consumed (mAh)
    double getCumulativeEnergy() const { return cumulativeEnergy_mAh; }

    // Get energy time-series
    const std::vector<TimeSample<double>>& getEnergyTimeSeries() const {
        return energyTimeSeries;
    }

    // === PDR Tracking (Per-Link) ===

    // Called when transmitting a packet (originated or forwarded)
    void recordPacketTransmission(
        GlobalPacketID packetId,
        uint16_t receiverId,
        int64_t timestamp
    );

    // Called when receiving ACK for a transmitted packet
    void recordAckReception(
        GlobalPacketID packetId,
        int64_t timestamp
    );

    // Get PDR (unified for originated + forwarded packets)
    double getPDR() const;

    // Get number of packets sent and ACKed (for debugging)
    size_t getTotalPacketsSent() const { return allTransmissions.size(); }
    size_t getTotalAcksReceived() const;

    // === Latency Tracking (End-to-End) ===

    // Called by MetricsAggregator when packet reaches C3
    void recordSuccessfulDelivery(
        GlobalPacketID packetId,
        int64_t latency_ms,
        int64_t interactionTimestamp,  // When this node sent/received the packet
        int64_t deliveryTimestamp      // When C3 received it
    );

    // Get latency statistics
    LatencyStats getLatencyStats() const;

    // Get all latency records
    const std::vector<LatencyRecord>& getLatencyRecords() const {
        return latencyRecords;
    }

private:
    // Node identification
    uint16_t nodeId;
    NodeClass nodeClass;

    // Logger reference
    Logger& logger;

    // === Energy State ===
    NodeState currentState;
    int64_t stateEntryTime;
    double cumulativeEnergy_mAh;
    std::vector<TimeSample<double>> energyTimeSeries;

    // Calculate energy consumed in a given state for a duration
    double calculateStateEnergy(NodeState state, int64_t duration_ms) const;

    // === PDR State ===
    struct TransmissionRecord {
        GlobalPacketID packetId;
        uint16_t receiverId;
        int64_t sendTime;
        bool ackReceived;
    };

    std::map<GlobalPacketID, TransmissionRecord> allTransmissions;

    // === Latency State ===
    std::vector<LatencyRecord> latencyRecords;

    // Helper functions for statistics
    LatencyStats computeLatencyStats(const std::vector<LatencyRecord>& records) const;
};
