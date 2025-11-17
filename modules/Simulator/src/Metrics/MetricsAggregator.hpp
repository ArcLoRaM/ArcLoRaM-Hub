#pragma once

#include "MetricsTypes.hpp"
#include "NodeMetrics.hpp"
#include <map>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <utility>

// Forward declarations
class NodeMetrics;
class Logger;

class MetricsAggregator {
public:
    explicit MetricsAggregator(Logger& logger);
    ~MetricsAggregator() = default;

    // === Node Registration ===

    // Register a node's metrics for aggregation
    void registerNodeMetrics(NodeMetrics* metrics);

    // === Global Packet ID Management ===

    // Assign a new globally unique packet ID, useful to track packets across nodes, for latency measurements, etc.
    GlobalPacketID assignGlobalPacketId();

    // Register packet key mapping: (nodeId, localId) -> globalId
    // Used to correlate ACKs back to original transmissions
    void registerPacketKey(uint16_t nodeId, uint16_t localPacketId, GlobalPacketID globalId);

    // Lookup global packet ID from (nodeId, localId)
    // Returns 0 if not found
    GlobalPacketID lookupGlobalPacketId(uint16_t nodeId, uint16_t localPacketId) const;

    // === Packet Chain Linking (for Multi-Hop Tracking) ===

    // Record that a packet was originated by a node
    void recordPacketOriginated(
        uint16_t nodeId,
        GlobalPacketID globalId,
        int64_t timestamp
    );

    // Record that a node forwarded a received packet as a new packet
    // Links receivedGlobalId -> forwardedGlobalId in the chain
    void recordPacketForwarded(
        uint16_t forwarderNodeId,
        GlobalPacketID receivedGlobalId,
        GlobalPacketID forwardedGlobalId,
        int64_t timestamp
    );

    // === C3 Delivery Handling ===

    // Called when C3 receives a packet - computes end-to-end latency for all nodes in path
    void recordPacketDeliveredToC3(
        GlobalPacketID globalId,
        int64_t timestamp
    );

    // === Time-Series Sampling ===

    // Sample energy metrics from all registered nodes (called by Clock)
    void sampleAllNodes(int64_t timestamp);

    // Get/Set sampling interval
    unsigned int getSamplingInterval() const { return samplingIntervalTicks; }
    void setSamplingInterval(unsigned int interval) { samplingIntervalTicks = interval; }

    // === Statistics Export (Future) ===

    // Get reference to all registered node metrics (for manual queries)
    const std::vector<NodeMetrics*>& getAllNodeMetrics() const {
        return allNodeMetrics;
    }

    // Log summary of all metrics for debugging/testing
    void logMetricsSummary() const;

private:
    // Global packet ID counter
    std::atomic<GlobalPacketID> nextGlobalPacketId;

    // Packet key mapping: (nodeId, localId) -> globalId
    struct PacketKey {
        uint16_t nodeId;
        uint16_t localPacketId;

        bool operator<(const PacketKey& other) const {
            if (nodeId != other.nodeId) return nodeId < other.nodeId;
            return localPacketId < other.localPacketId;
        }
    };
    mutable std::mutex packetKeyMutex;
    std::map<PacketKey, GlobalPacketID> packetKeyToGlobalId;

    // Packet lifecycle tracking
    struct PacketLifecycle {
        GlobalPacketID globalId;
        uint16_t originNodeId;
        int64_t originTimestamp;

        struct HopInfo {
            uint16_t nodeId;
            int64_t interactionTime;  // When this node sent/received the packet
        };
        std::vector<HopInfo> pathTrace;
    };
    mutable std::mutex lifecycleMutex;
    std::map<GlobalPacketID, PacketLifecycle> packetRegistry;

    // Forward chain: forwardedGlobalId -> receivedGlobalId
    // Used to walk back from C3 delivery to origin
    std::map<GlobalPacketID, GlobalPacketID> forwardChain;

    // Registered node metrics
    std::vector<NodeMetrics*> allNodeMetrics;

    // Sampling configuration
    unsigned int samplingIntervalTicks;

    // Logger reference
    Logger& logger;

    // Helper: Walk back the forwarding chain to find origin packet
    GlobalPacketID findOriginPacket(GlobalPacketID currentId) const;

    // Helper: Build full path trace from origin to current packet
    std::vector<GlobalPacketID> buildPacketChain(GlobalPacketID currentId) const;
};
