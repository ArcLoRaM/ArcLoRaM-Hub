#include "MetricsManager.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

// ============================================================================
// NodeMetricsData Implementation
// ============================================================================

double NodeMetricsData::computePDR() const {
    if (totalPacketsSent == 0) {
        return 0.0;
    }
    return (static_cast<double>(totalAcksReceived) / static_cast<double>(totalPacketsSent)) * 100.0;
}

LatencyStats NodeMetricsData::computeLatencyStats() const {
    LatencyStats stats;

    if (latencyRecords.empty()) {
        return stats;
    }

    // Extract latency values and sort them
    std::vector<int64_t> latencies;
    latencies.reserve(latencyRecords.size());
    for (const auto& record : latencyRecords) {
        latencies.push_back(record.latency_ms);
    }
    std::sort(latencies.begin(), latencies.end());

    stats.sampleCount = latencies.size();

    // Min and Max
    stats.min = static_cast<double>(latencies.front());
    stats.max = static_cast<double>(latencies.back());

    // Mean
    int64_t sum = std::accumulate(latencies.begin(), latencies.end(), 0LL);
    stats.mean = static_cast<double>(sum) / static_cast<double>(latencies.size());

    // Median
    size_t mid = latencies.size() / 2;
    if (latencies.size() % 2 == 0) {
        stats.median = (static_cast<double>(latencies[mid - 1]) + static_cast<double>(latencies[mid])) / 2.0;
    } else {
        stats.median = static_cast<double>(latencies[mid]);
    }

    // 95th percentile
    size_t p95_idx = static_cast<size_t>(std::ceil(0.95 * latencies.size())) - 1;
    if (p95_idx >= latencies.size()) {
        p95_idx = latencies.size() - 1;
    }
    stats.p95 = static_cast<double>(latencies[p95_idx]);

    return stats;
}

double NodeMetricsData::getTotalEnergy() const {
    double total = 0.0;
    for (const auto& sample : energySamples) {
        total += sample.second;  // sample.second is energy in mJ
    }
    return total;
}

// ============================================================================
// MetricsManager Implementation
// ============================================================================

MetricsManager::MetricsManager() {
    std::cout << "[MetricsManager] Initialized" << std::endl;
}

void MetricsManager::updateNodePDR(int nodeId, int tickNb, int totalSent, int totalAcked) {
    std::lock_guard<std::mutex> lock(metricsMutex);

    NodeMetricsData& data = getOrCreateNodeMetrics(nodeId);
    data.lastUpdateTick = tickNb;
    data.totalPacketsSent = totalSent;
    data.totalAcksReceived = totalAcked;

    double pdr = data.computePDR();

    std::cout << "[MetricsManager] Node " << nodeId
              << " PDR Update @ tick " << tickNb
              << ": Sent=" << totalSent
              << ", Acked=" << totalAcked
              << ", PDR=" << pdr << "%"
              << std::endl;
}

void MetricsManager::addLatencyRecords(int nodeId, int tickNb, const std::vector<LatencyRecord>& records) {
    if (records.empty()) {
        return;  // No records to add
    }

    std::lock_guard<std::mutex> lock(metricsMutex);

    NodeMetricsData& data = getOrCreateNodeMetrics(nodeId);
    data.lastUpdateTick = tickNb;

    // Append new records to existing ones
    data.latencyRecords.insert(data.latencyRecords.end(), records.begin(), records.end());

    std::cout << "[MetricsManager] Node " << nodeId
              << " Latency Records @ tick " << tickNb
              << ": Added " << records.size() << " new records"
              << " (Total: " << data.latencyRecords.size() << ")"
              << std::endl;

    // Log details of each new record
    for (const auto& record : records) {
        std::cout << "  - PacketID=" << record.packetId
                  << ", Latency=" << record.latency_ms << "ms"
                  << ", Origin=" << record.originTimestamp
                  << ", Delivery=" << record.deliveryTimestamp
                  << std::endl;
    }
}

void MetricsManager::addEnergySamples(int nodeId, int tickNb, const std::vector<std::pair<int, double>>& samples) {
    if (samples.empty()) {
        return;  // No samples to add
    }

    std::lock_guard<std::mutex> lock(metricsMutex);

    NodeMetricsData& data = getOrCreateNodeMetrics(nodeId);
    data.lastUpdateTick = tickNb;

    // Append new samples to existing ones
    data.energySamples.insert(data.energySamples.end(), samples.begin(), samples.end());

    double newEnergy = 0.0;
    for (const auto& sample : samples) {
        newEnergy += sample.second;
    }

    std::cout << "[MetricsManager] Node " << nodeId
              << " Energy Samples @ tick " << tickNb
              << ": Added " << samples.size() << " new samples"
              << " (Total: " << data.energySamples.size() << ")"
              << ", New Energy=" << newEnergy << "mJ"
              << ", Total Energy=" << data.getTotalEnergy() << "mJ"
              << std::endl;

    // Log details of each new sample
    for (const auto& sample : samples) {
        std::cout << "  - Tick=" << sample.first
                  << ", Energy=" << sample.second << "mJ"
                  << std::endl;
    }
}

std::optional<NodeMetricsData> MetricsManager::getNodeMetrics(int nodeId) const {
    std::lock_guard<std::mutex> lock(metricsMutex);

    auto it = nodeMetrics.find(nodeId);
    if (it != nodeMetrics.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<int> MetricsManager::getAllNodeIds() const {
    std::lock_guard<std::mutex> lock(metricsMutex);

    std::vector<int> nodeIds;
    nodeIds.reserve(nodeMetrics.size());

    for (const auto& pair : nodeMetrics) {
        nodeIds.push_back(pair.first);
    }

    return nodeIds;
}

void MetricsManager::clearAllMetrics() {
    std::lock_guard<std::mutex> lock(metricsMutex);

    size_t nodeCount = nodeMetrics.size();
    nodeMetrics.clear();

    std::cout << "[MetricsManager] Cleared metrics for " << nodeCount << " nodes" << std::endl;
}

size_t MetricsManager::getNodeCount() const {
    std::lock_guard<std::mutex> lock(metricsMutex);
    return nodeMetrics.size();
}

NodeMetricsData& MetricsManager::getOrCreateNodeMetrics(int nodeId) {
    // NOTE: This method is NOT thread-safe and must be called with metricsMutex locked

    auto it = nodeMetrics.find(nodeId);
    if (it != nodeMetrics.end()) {
        return it->second;
    }

    // Create new entry
    NodeMetricsData newData;
    newData.nodeId = nodeId;

    auto result = nodeMetrics.emplace(nodeId, newData);

    std::cout << "[MetricsManager] Created new metrics entry for node " << nodeId << std::endl;

    return result.first->second;
}
