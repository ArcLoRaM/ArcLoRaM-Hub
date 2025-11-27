#include "NodeMetrics.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

NodeMetrics::NodeMetrics(uint16_t nodeId, NodeClass nodeClass, Logger& logger)
    : nodeId(nodeId),
      nodeClass(nodeClass),
      logger(logger),
      currentState(NodeState::Sleeping),  // Default initial state
      stateEntryTime(0),
      cumulativeEnergy_mAh(0.0)
{
}

// === Energy Tracking ===

void NodeMetrics::onStateChange(NodeState newState, int64_t timestamp) {
    // Calculate energy consumed in previous state
    int64_t duration_ms = timestamp - stateEntryTime;
    if (duration_ms > 0) {
        double energyDelta = calculateStateEnergy(currentState, duration_ms);
        cumulativeEnergy_mAh += energyDelta;
    }
    else {
        logger.logWarning("NodeMetrics: Node " + std::to_string(nodeId)
            + " state change called with non-positive duration: "
            + std::to_string(duration_ms) + " ms");
    }

    // Update to new state
    currentState = newState;
    stateEntryTime = timestamp;
}

void NodeMetrics::sampleEnergyTimeSeries(int64_t timestamp) {
    // Add current cumulative energy to time-series
    energyTimeSeries.push_back({timestamp, cumulativeEnergy_mAh});
}

double NodeMetrics::calculateStateEnergy(NodeState state, int64_t duration_ms) const {
    // Look up current draw for this state
    auto it = MetricsConfig::STATE_CURRENT_DRAW_MA.find(state);
    if (it == MetricsConfig::STATE_CURRENT_DRAW_MA.end()) {
        return 0.0;  // Unknown state
    }

    double current_mA = it->second;
    double hours = duration_ms / (1000.0 * 3600.0);  // Convert ms to hours
    return current_mA * hours;  // mAh
}

// === PDR Tracking ===

void NodeMetrics::recordPacketTransmission(
    GlobalPacketID packetId,
    uint16_t receiverId,
    int64_t timestamp)
{
    TransmissionRecord record{packetId, receiverId, timestamp, false};
    allTransmissions[packetId] = record;

    // logger.logDebug("NodeMetrics: Node " + std::to_string(nodeId)
    //     + " transmitted packet " + std::to_string(packetId)
    //     + " to receiver " + std::to_string(receiverId)
    //     + " at " + std::to_string(timestamp) + " ms");
}

void NodeMetrics::recordAckReception(
    GlobalPacketID packetId,
    int64_t timestamp)
{
    auto it = allTransmissions.find(packetId);
    if (it != allTransmissions.end()) {
        it->second.ackReceived = true;

        // logger.logDebug("NodeMetrics: Node " + std::to_string(nodeId)
        //     + " received ACK for packet " + std::to_string(packetId)
        //     + " at " + std::to_string(timestamp) + " ms"
        //     + " | Current PDR: " + std::to_string(getPDR() * 100.0) + "%");
    } else {
        logger.logWarning("NodeMetrics: Node " + std::to_string(nodeId)
            + " received ACK for unknown packet " + std::to_string(packetId));
    }
}

double NodeMetrics::getPDR() const {
    if (allTransmissions.empty()) {
        return 0.0;
    }

    size_t ackedCount = std::count_if(
        allTransmissions.begin(),
        allTransmissions.end(),
        [](const auto& pair) { return pair.second.ackReceived; }
    );

    return static_cast<double>(ackedCount) / allTransmissions.size();
}

size_t NodeMetrics::getTotalAcksReceived() const {
    return std::count_if(
        allTransmissions.begin(),
        allTransmissions.end(),
        [](const auto& pair) { return pair.second.ackReceived; }
    );
}

// === Latency Tracking ===

void NodeMetrics::recordSuccessfulDelivery(
    GlobalPacketID packetId,
    int64_t latency_ms,
    int64_t interactionTimestamp,
    int64_t deliveryTimestamp)
{
    LatencyRecord record{
        packetId,
        latency_ms,
        interactionTimestamp,
        deliveryTimestamp
    };
    latencyRecords.push_back(record);
}

LatencyStats NodeMetrics::getLatencyStats() const {
    return computeLatencyStats(latencyRecords);
}

LatencyStats NodeMetrics::computeLatencyStats(const std::vector<LatencyRecord>& records) const {
    
    LatencyStats stats;

    if (records.empty()) {
        return stats;
    }

    stats.sampleCount = records.size();

    // Extract latencies
    std::vector<int64_t> latencies;
    latencies.reserve(records.size());
    for (const auto& record : records) {
        latencies.push_back(record.latency_ms);
    }

    // Sort for percentile calculations
    std::vector<int64_t> sortedLatencies = latencies;
    std::sort(sortedLatencies.begin(), sortedLatencies.end());

    // Min and Max
    stats.min = static_cast<double>(sortedLatencies.front());
    stats.max = static_cast<double>(sortedLatencies.back());

    // Mean
    int64_t sum = std::accumulate(sortedLatencies.begin(), sortedLatencies.end(), 0LL);
    stats.mean = static_cast<double>(sum) / sortedLatencies.size();

    // Median (50th percentile)
    size_t medianIdx = sortedLatencies.size() / 2;
    if (sortedLatencies.size() % 2 == 0) {
        stats.median = (sortedLatencies[medianIdx - 1] + sortedLatencies[medianIdx]) / 2.0;
    } else {
        stats.median = static_cast<double>(sortedLatencies[medianIdx]);
    }

    // 95th percentile
    size_t p95Idx = static_cast<size_t>(std::ceil(0.95 * sortedLatencies.size())) - 1;
    stats.p95 = static_cast<double>(sortedLatencies[p95Idx]);

    return stats;
}

// === Incremental Export Tracking ===

std::vector<LatencyRecord> NodeMetrics::getNewLatencyRecords() {
    std::vector<LatencyRecord> newRecords;
    for (size_t i = lastExportedLatencyIndex; i < latencyRecords.size(); ++i) {
        newRecords.push_back(latencyRecords[i]);
    }
    return newRecords;
}

std::vector<TimeSample<double>> NodeMetrics::getNewEnergySamples() {
    std::vector<TimeSample<double>> newSamples;
    for (size_t i = lastExportedEnergyIndex; i < energyTimeSeries.size(); ++i) {
        newSamples.push_back(energyTimeSeries[i]);
    }
    return newSamples;
}

void NodeMetrics::markLatencyExported() {
    lastExportedLatencyIndex = latencyRecords.size();
}

void NodeMetrics::markEnergyExported() {
    lastExportedEnergyIndex = energyTimeSeries.size();
}
