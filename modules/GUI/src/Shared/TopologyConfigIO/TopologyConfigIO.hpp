#pragma once // Recommended or use include guards


#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>
#include "../../Screens/TopologyEditorPackage/TopologyEditorState.hpp"
#include "../../Screens/ProtocolVisualisationPackage/States/SimulationConfiguration.hpp"
#include "../../Objects/Device/Device.hpp"


class TopologyConfigIO {
public:
    // Write current topology to a file
    static bool write(const std::string& path,
                      const std::unordered_map<int, std::unique_ptr<Device>>& nodes,
                      const std::unordered_map<int, std::unordered_set<int>>& routings
                      );
    // Used in the topology editor
   static bool read(const std::string& path, TopologyEditorState& state);


    //used in the protocol visualiser.
static bool readTopologyConfig(const std::string& path, SimulationConfiguration& state);

private:

    static std::optional<std::pair<int, int>> computeRoutingInfo(
                int startId,
                const std::unordered_map<int, std::unordered_set<int>>& routings,
                const std::unordered_map<int, std::unique_ptr<Device>>& nodes);   
    
    static bool validateConfigFile(std::istream& in, std::string* outText = nullptr, TopologyEditorState* outState = nullptr);

 };