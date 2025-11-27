#ifndef METRICSEXPORTER_HPP
#define METRICSEXPORTER_HPP

#include "MetricsAggregator.hpp"
#include "../Connectivity/Logger/Logger.hpp"

class MetricsExporter {
public:
    MetricsExporter() = default;
    ~MetricsExporter() = default;

    // Export all metrics to GUI via TCP
    void exportAllMetrics(MetricsAggregator* aggregator, int currentTick, Logger& logger);

private:
    // Send PDR metrics for a single node
    void sendNodePDRMetrics(NodeMetrics* metrics, int tick, Logger& logger);

    // Send new latency records for a single node
    void sendLatencyRecords(NodeMetrics* metrics, int tick, Logger& logger);

    // Send new energy samples for a single node
    void sendEnergySamples(NodeMetrics* metrics, int tick, Logger& logger);
};

#endif // METRICSEXPORTER_HPP
