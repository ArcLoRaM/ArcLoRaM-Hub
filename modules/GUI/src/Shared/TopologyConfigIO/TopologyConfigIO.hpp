#pragma once // Recommended or use include guards


#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include "../../Screens/TopologyEditorPackage/TopologyEditorState.hpp"
#include "../../Screens/ProtocolVisualisationPackage/TopologyVisualisationState.hpp"
#include "../../Visualisation/Device/Device.hpp"


class TopologyConfigIO {
public:
    // Write current topology to a file
    static bool write(const std::string& path,
                      const std::unordered_map<int, std::unique_ptr<Device>>& nodes,
                      const std::unordered_map<int, std::unordered_set<int>>& routings,
                      TopologyMode mode);
    // Placeholder for future extension
   static bool read(const std::string& path, TopologyEditorState& state);
    static std::optional<std::string> readToString(const std::string& path);

static bool readToVisualisationState(const std::string& path, TopologyVisualisationState& state);

private:

    static std::optional<std::pair<int, int>> computeRoutingInfo(
                int startId,
                const std::unordered_map<int, std::unordered_set<int>>& routings,
                const std::unordered_map<int, std::unique_ptr<Device>>& nodes);   
    
    static bool validateConfigFile(std::istream& in, std::string* outText = nullptr, TopologyEditorState* outState = nullptr);

 };