#pragma once

#include <string>

class ProtocolVisualiser;
class ProtocolVisualisationState;

class CsvMetricWriter {
public:
void writeNetworkMetricsToCsv(const std::string& filename,
                                               const ProtocolVisualiser& visualiser,
                                               const ProtocolVisualisationState& state);
};
