#pragma once

#include <string>

class ProtocolVisualiser;
class SimulationConfiguration;

class CsvMetricWriter {
public:
void writeNetworkMetricsToCsv(const std::string& filename,
                                               const ProtocolVisualiser& visualiser,
                                               const SimulationConfiguration& state);
};
