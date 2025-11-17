#include "MetricsAggregator.hpp"
#include "MetricsConfig.hpp"
#include "../Connectivity/Logger/Logger.hpp"
#include <algorithm>

MetricsAggregator::MetricsAggregator(Logger& logger)
    : nextGlobalPacketId(1),  // Start from 1 (0 reserved for "not found")
      samplingIntervalTicks(MetricsConfig::DEFAULT_SAMPLING_INTERVAL_TICKS),
      logger(logger)
{
}

// === Node Registration ===

void MetricsAggregator::registerNodeMetrics(NodeMetrics* metrics) {
    if (metrics) {
        allNodeMetrics.push_back(metrics);
    }
}

// === Global Packet ID Management ===

GlobalPacketID MetricsAggregator::assignGlobalPacketId() {
    return nextGlobalPacketId.fetch_add(1, std::memory_order_relaxed);
}

void MetricsAggregator::registerPacketKey(
    uint16_t nodeId,
    uint16_t localPacketId,
    GlobalPacketID globalId)
{
    std::lock_guard<std::mutex> lock(packetKeyMutex);
    PacketKey key{nodeId, localPacketId};
    packetKeyToGlobalId[key] = globalId;
}

GlobalPacketID MetricsAggregator::lookupGlobalPacketId(
    uint16_t nodeId,
    uint16_t localPacketId) const
{
    std::lock_guard<std::mutex> lock(packetKeyMutex);
    PacketKey key{nodeId, localPacketId};
    auto it = packetKeyToGlobalId.find(key);
    if (it != packetKeyToGlobalId.end()) {
        return it->second;
    }
    logger.logWarning("MetricsAggregator: Lookup failed for Node "
        + std::to_string(nodeId)
        + ", Local Packet ID " + std::to_string(localPacketId));
    return 0;  // Not found
}

// === Packet Chain Linking ===

void MetricsAggregator::recordPacketOriginated(
    uint16_t nodeId,
    GlobalPacketID globalId,
    int64_t timestamp)
{
    std::lock_guard<std::mutex> lock(lifecycleMutex);

    PacketLifecycle lifecycle;
    lifecycle.globalId = globalId;
    lifecycle.originNodeId = nodeId;
    lifecycle.originTimestamp = timestamp;

    // Add origin node to path trace
    lifecycle.pathTrace.push_back({nodeId, timestamp});

    packetRegistry[globalId] = lifecycle;
}

void MetricsAggregator::recordPacketForwarded(
    uint16_t forwarderNodeId,
    GlobalPacketID receivedGlobalId,
    GlobalPacketID forwardedGlobalId,
    int64_t timestamp)
{
    std::lock_guard<std::mutex> lock(lifecycleMutex);

    // Link the chain: forwarded -> received
    forwardChain[forwardedGlobalId] = receivedGlobalId;

    // Create lifecycle entry for forwarded packet
    // Copy origin info from received packet
    auto receivedIt = packetRegistry.find(receivedGlobalId);
    if (receivedIt != packetRegistry.end()) {
        PacketLifecycle forwardedLifecycle;
        forwardedLifecycle.globalId = forwardedGlobalId;
        forwardedLifecycle.originNodeId = receivedIt->second.originNodeId;
        forwardedLifecycle.originTimestamp = receivedIt->second.originTimestamp;

        // Copy path from received packet
        forwardedLifecycle.pathTrace = receivedIt->second.pathTrace;

        // Add forwarder to path
        forwardedLifecycle.pathTrace.push_back({forwarderNodeId, timestamp});

        packetRegistry[forwardedGlobalId] = forwardedLifecycle;
    }
}

// === C3 Delivery Handling ===

void MetricsAggregator::recordPacketDeliveredToC3(
    GlobalPacketID globalId,
    int64_t timestamp)
{
    std::lock_guard<std::mutex> lock(lifecycleMutex);

    // Get the packet lifecycle
    auto it = packetRegistry.find(globalId);
    if (it == packetRegistry.end()) {
        // Packet not found in registry - this shouldn't happen
        logger.logWarning("MetricsAggregator: Delivered packet ID " + std::to_string(globalId) + " not found in registry.");
        return;
    }

    const PacketLifecycle& lifecycle = it->second;

    // Update latency metrics for ALL nodes in the path
    for (const auto& hop : lifecycle.pathTrace) {
        // Find the corresponding NodeMetrics by node ID
        auto nodeMetricsIt = std::find_if(
            allNodeMetrics.begin(),
            allNodeMetrics.end(),
            [hopNodeId = hop.nodeId](NodeMetrics* metrics) {
                return metrics && metrics->getNodeId() == hopNodeId;
            }
        );

        if (nodeMetricsIt != allNodeMetrics.end()) {
            NodeMetrics* nodeMetrics = *nodeMetricsIt;

            // Calculate latency from this node's interaction to C3 delivery
            int64_t latency_ms = timestamp - hop.interactionTime;

            
            // Record the successful delivery
            nodeMetrics->recordSuccessfulDelivery(
                globalId,
                latency_ms,
                hop.interactionTime,
                timestamp
            );

            logger.logDebug("MetricsAggregator: Recorded delivery for Node "
                + std::to_string(hop.nodeId)
                + " | Packet ID: " + std::to_string(globalId)
                + " | Latency: " + std::to_string(latency_ms) + " ms"
            );
        }
    }
}

// === Time-Series Sampling ===

void MetricsAggregator::sampleAllNodes(int64_t timestamp) {
    for (NodeMetrics* metrics : allNodeMetrics) {
        if (metrics) {
            metrics->sampleEnergyTimeSeries(timestamp);
        }
    }
}

// === Helper Functions ===

GlobalPacketID MetricsAggregator::findOriginPacket(GlobalPacketID currentId) const {
    GlobalPacketID originId = currentId;

    // Walk back the chain until we reach the origin (no more parents)
    while (forwardChain.find(originId) != forwardChain.end()) {
        originId = forwardChain.at(originId);
    }

    return originId;
}

std::vector<GlobalPacketID> MetricsAggregator::buildPacketChain(GlobalPacketID currentId) const {
    std::vector<GlobalPacketID> chain;

    // Walk back to origin
    GlobalPacketID id = currentId;
    chain.push_back(id);

    while (forwardChain.find(id) != forwardChain.end()) {
        id = forwardChain.at(id);
        chain.push_back(id);
    }

    // Reverse to get origin -> current order
    std::reverse(chain.begin(), chain.end());

    return chain;
}

// === Logging ===

void MetricsAggregator::logMetricsSummary() const {
    logger.logSystem("=== METRICS SUMMARY ===");
    logger.logSystem("Total nodes tracked: " + std::to_string(allNodeMetrics.size()));

    for (const NodeMetrics* metrics : allNodeMetrics) {
        if (!metrics) continue;

        uint16_t nodeId = metrics->getNodeId();
        NodeClass nodeClass = metrics->getNodeClass();
        logger.logSystem("--- Node " + std::to_string(nodeId) + ", Class " + std::to_string(static_cast<int>(nodeClass)) + " ---");

        // Energy metrics
        double energy = metrics->getCumulativeEnergy();
        size_t energySamples = metrics->getEnergyTimeSeries().size();
        logger.logSystem("  Energy: " + std::to_string(energy) + " mAh"
            + " (" + std::to_string(energySamples) + " samples)");

        // PDR metrics
        size_t totalSent = metrics->getTotalPacketsSent();
        size_t totalAcked = metrics->getTotalAcksReceived();
        double pdr = metrics->getPDR();
        logger.logSystem("  PDR: " + std::to_string(pdr * 100.0) + "%"
            + " (" + std::to_string(totalAcked) + "/" + std::to_string(totalSent) + " ACKed)");

        // Latency metrics
        LatencyStats latencyStats = metrics->getLatencyStats();
        if (latencyStats.sampleCount > 0) {
            logger.logSystem("  Latency: "
                + std::to_string(latencyStats.sampleCount) + " deliveries"
                + " | Mean: " + std::to_string(latencyStats.mean) + " ms"
                + " | Median: " + std::to_string(latencyStats.median) + " ms"
                + " | P95: " + std::to_string(latencyStats.p95) + " ms"
                + " | Min: " + std::to_string(latencyStats.min) + " ms"
                + " | Max: " + std::to_string(latencyStats.max) + " ms");
        } else {
            logger.logSystem("  Latency: No deliveries recorded");
        }
    }

    logger.logSystem("=== END METRICS SUMMARY ===");
}
