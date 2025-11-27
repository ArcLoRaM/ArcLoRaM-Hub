#ifndef METRICSMANAGER_HPP
#define METRICSMANAGER_HPP

#include <unordered_map>
#include <vector>
#include <mutex>
#include <optional>
#include "../Metrics/MetricsTypes.hpp"

// Forward declaration
struct LatencyRecord;

/**
 * @brief Stores all metrics data for a single node
 *
 * Contains both snapshot metrics (PDR) that are updated each tick,
 * and time-series metrics (latency, energy) that accumulate over time.
 */
struct NodeMetricsData {
    int nodeId;
    int lastUpdateTick = 0;

    // PDR Metrics - updated/overwritten each export
    int totalPacketsSent = 0;
    int totalAcksReceived = 0;

    // Time-Series Data - accumulated over time
    std::vector<LatencyRecord> latencyRecords;
    std::vector<std::pair<int, double>> energySamples;  // <tick, energy_mJ>

    /**
     * @brief Compute Packet Delivery Ratio
     * @return PDR as percentage (0.0 to 100.0), or 0.0 if no packets sent
     */
    double computePDR() const;

    /**
     * @brief Compute latency statistics from accumulated records
     * @return LatencyStats with mean, median, percentiles, etc.
     */
    LatencyStats computeLatencyStats() const;

    /**
     * @brief Get total energy consumed (sum of all samples)
     * @return Total energy in millijoules
     */
    double getTotalEnergy() const;
};

/**
 * @brief Thread-safe manager for node metrics data
 *
 * Receives metrics packets from the simulator and stores them
 * in per-node data structures. Provides thread-safe access for
 * UI display and data export.
 */
class MetricsManager {
public:
    MetricsManager();
    ~MetricsManager() = default;

    /**
     * @brief Update PDR metrics for a node (snapshot data)
     * @param nodeId Node identifier
     * @param tickNb Current simulation tick
     * @param totalSent Total packets sent (cumulative)
     * @param totalAcked Total acknowledgments received (cumulative)
     */
    void updateNodePDR(int nodeId, int tickNb, int totalSent, int totalAcked);

    /**
     * @brief Add new latency records for a node (time-series data)
     * @param nodeId Node identifier
     * @param tickNb Current simulation tick
     * @param records Vector of new LatencyRecord entries
     */
    void addLatencyRecords(int nodeId, int tickNb, const std::vector<LatencyRecord>& records);

    /**
     * @brief Add new energy samples for a node (time-series data)
     * @param nodeId Node identifier
     * @param tickNb Current simulation tick
     * @param samples Vector of new energy samples (tick, energy_mJ)
     */
    void addEnergySamples(int nodeId, int tickNb, const std::vector<std::pair<int, double>>& samples);

    /**
     * @brief Get metrics data for a specific node
     * @param nodeId Node identifier
     * @return Optional containing NodeMetricsData if node exists
     */
    std::optional<NodeMetricsData> getNodeMetrics(int nodeId) const;

    /**
     * @brief Get all node IDs that have metrics data
     * @return Vector of node IDs
     */
    std::vector<int> getAllNodeIds() const;

    /**
     * @brief Clear all metrics data (e.g., on simulation restart)
     */
    void clearAllMetrics();

    /**
     * @brief Get the total number of nodes with metrics
     * @return Count of nodes
     */
    size_t getNodeCount() const;

private:
    // Thread-safe storage of per-node metrics
    std::unordered_map<int, NodeMetricsData> nodeMetrics;
    mutable std::mutex metricsMutex;

    // Helper to get or create node metrics entry (not thread-safe, must be called with lock)
    NodeMetricsData& getOrCreateNodeMetrics(int nodeId);
};

#endif // METRICSMANAGER_HPP
