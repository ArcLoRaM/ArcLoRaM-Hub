#pragma once

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "DeviceManager.hpp"

class RoutingManager
{
public:
    void addRouting(int fromId, int toId);
    void removeRouting(int fromId, int toId);

    std::unordered_map<int, std::unordered_set<int>> getRoutingTable() const;
    void drawRootings(tgui::CanvasSFML::Ptr canvas);

    void setDeviceManager(DeviceManager *dm) { deviceManager = dm; }

    void clear();

private:
    mutable std::mutex mutex;
    std::unordered_map<int, std::unordered_set<int>> routingTable;

    DeviceManager *deviceManager = nullptr;
};
