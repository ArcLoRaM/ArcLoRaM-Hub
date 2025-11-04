#include "RoutingManager.hpp"
#include "../../Shared/Helper.hpp"

void RoutingManager::addRouting(int fromId, int toId)
{    // ID1 ---> ID2 , path towards ID2
    std::lock_guard<std::mutex> lock(mutex);
    if(!deviceManager)
    {
        std::cout << "********Device Manager not set in RoutingManager. Cannot add routing.********\n";
        return;
    }
    if (deviceManager->containsDevice(fromId) && deviceManager->containsDevice(toId))
    {
        routingTable[fromId].insert(toId); // Only store toId in fromId's adjacency list
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
    }
}

void RoutingManager::removeRouting(int fromId, int toId)
{
        std::lock_guard<std::mutex> lock(mutex);
    if(!deviceManager)
    {
        std::cout << "********Device Manager not set in RoutingManager. Cannot remove routing.********\n";
        return;
    }
    if (deviceManager->containsDevice(fromId) && deviceManager->containsDevice(toId))
    {
        routingTable[fromId].erase(toId);
    }
    else
    {
        std::cout << "********One or both devices do not exist.********\n";
    }
}

std::unordered_map<int, std::unordered_set<int>> RoutingManager::getRoutingTable() const
{
    return std::unordered_map<int, std::unordered_set<int>>();
}

void RoutingManager::drawRootings(tgui::CanvasSFML::Ptr canvas)
{ for (const auto &[device, connectedDevices] : routingTable)
    {

        for (const auto &connectedDevice : connectedDevices)
        {
            if (deviceManager)
            {
                auto positionsOpt = deviceManager->getDevicesPosition(device, connectedDevice);
                if (positionsOpt.has_value())
                {
                    auto [start, end] = positionsOpt.value();
                    drawArrowWithHeads(canvas, start, end, 35.f);
                }
            }
        }
    }
}
