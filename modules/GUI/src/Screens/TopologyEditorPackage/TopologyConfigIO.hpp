#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <optional>

// Forward declarations
class Device;
enum class TopologyMode;


class TopologyConfigIO {
public:
    // Write current topology to a file
    static void write(const std::string& path,
                      const std::unordered_map<int, std::unique_ptr<Device>>& nodes,
                      const std::unordered_map<int, std::unordered_set<int>>& routings,
                      TopologyMode mode);
    // Placeholder for future extension
    static bool read(const std::string& path, TopologyEditorState& state);

private:

    static std::optional<std::pair<int, int>> computeRoutingInfo(
                int startId,
                const std::unordered_map<int, std::unordered_set<int>>& routings,
                const std::unordered_map<int, std::unique_ptr<Device>>& nodes);   
 };