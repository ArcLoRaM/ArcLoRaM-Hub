#include "MetricsExporter.hpp"
#include "../Connectivity/TCP/Packets/Packets.hpp"

void MetricsExporter::exportAllMetrics(MetricsAggregator* aggregator,
                                       int currentTick,
                                       Logger& logger) {
    if (!aggregator) {
        return;
    }

    auto allMetrics = aggregator->getAllNodeMetrics();

    for (NodeMetrics* metrics : allMetrics) {
        if (!metrics) {
            continue;
        }

        // 1. Send PDR counters (always sent every export cycle)
        sendNodePDRMetrics(metrics, currentTick, logger);

        // 2. Send NEW latency records (only if new data exists)
        auto newLatency = metrics->getNewLatencyRecords();
        if (!newLatency.empty()) {
            sendLatencyRecords(metrics, currentTick, logger);
            metrics->markLatencyExported();
        }

        // 3. Send NEW energy samples (only if new data exists)
        auto newEnergy = metrics->getNewEnergySamples();
        if (!newEnergy.empty()) {
            sendEnergySamples(metrics, currentTick, logger);
            metrics->markEnergyExported();
        }
    }
}

void MetricsExporter::sendNodePDRMetrics(NodeMetrics* metrics, int tick, Logger& logger) {
    sf::Packet packet;
    nodeMetricsPacket pdrPacket(
        tick,
        metrics->getNodeId(),
        metrics->getTotalPacketsSent(),
        metrics->getTotalAcksReceived()
    );
    packet << pdrPacket;
    logger.sendTcpPacket(packet);
}

void MetricsExporter::sendLatencyRecords(NodeMetrics* metrics, int tick, Logger& logger) {
    auto newRecords = metrics->getNewLatencyRecords();

    sf::Packet packet;
    latencyRecordsPacket latencyPacket(tick, metrics->getNodeId(), newRecords);
    packet << latencyPacket;
    logger.sendTcpPacket(packet);
}

void MetricsExporter::sendEnergySamples(NodeMetrics* metrics, int tick, Logger& logger) {
    auto newSamples = metrics->getNewEnergySamples();

    // Convert TimeSample<double> to pair<int, double>
    std::vector<std::pair<int, double>> samples;
    samples.reserve(newSamples.size());
    for (const auto& sample : newSamples) {
        
        samples.emplace_back(sample.timestamp, sample.value);
    }

    sf::Packet packet;
    energySamplesPacket energyPacket(tick, metrics->getNodeId(), samples);
    packet << energyPacket;
    logger.sendTcpPacket(packet);
}
