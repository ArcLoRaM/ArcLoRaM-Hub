#pragma once

#include <string>

class VisualiserManager;
class ProtocolVisualisationState;

class CsvMetricWriter {
public:
    void writeNetworkMetricsToCsv(const std::string& filename,
                                  const VisualiserManager& manager,
                                  const ProtocolVisualisationState& state);
};
