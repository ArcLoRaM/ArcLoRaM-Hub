#include "DeviceManager.hpp"

void DeviceManager::addDevice(std::unique_ptr<Device> device)
{
    std::lock_guard<std::mutex> lock(devicesMutex);

    if (!device)
    {
        throw std::runtime_error("Error: Attempted to add a null device.");
    }
    int nodeId = device->getNodeId();

    if (devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << device->getNodeId() << " already exists." << std::endl;
        return; // Device already exists, do not add
    }

    devices[nodeId] = std::move(device);
}

void DeviceManager::update(InputManager &inputManager, tgui::Gui &gui, const tgui::CanvasSFML::Ptr &canvas)
{
        {
        std::lock_guard<std::mutex> lock(devicesMutex);
        for (auto &[id, device] : devices)
        {
            device->update(inputManager, gui, canvas);
        }
    }
}

void DeviceManager::draw( std::shared_ptr<tgui::CanvasSFML> &canvas)
{    {
        std::lock_guard<std::mutex> lock(devicesMutex);
        for (auto &[id, device] : devices)
        {
            device->draw(canvas);
        }
    }
}

void DeviceManager::updateDeviceState(int nodeId, DeviceState state)
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to update its state." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }

    devices[nodeId]->setState(state);
}

std::optional<sf::Vector2f> DeviceManager::getDevicePosition(int nodeId) const
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    auto it = devices.find(nodeId);
    if (it == devices.end())
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to find device coordinate." << std::endl;
        return std::nullopt; // Device does not exist
    }

    return { it->second->getCenteredPosition()};

}

void DeviceManager::incrementRetransmission(int nodeId)
{    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to add retransmission." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }
    devices[nodeId]->metrics.incrementRetransmission();

}

void DeviceManager::incrementPacketSent(int nodeId)
{    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to add PacketSent." << std::endl;
        return; // Device does not exist, do not increment retransmission
    }

    devices[nodeId]->metrics.incrementPacketSent();

}

void DeviceManager::incrementListeningData(int nodeId)
{    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Listening Data." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementListeningData();

}

void DeviceManager::incrementTransmittingData(int nodeId)
{    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Transmitting Data." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementTransmittingData();

}

void DeviceManager::incrementListeningAck(int nodeId)
{    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Listening Ack." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementListeningAck();

}

void DeviceManager::incrementTransmittingAck(int nodeId)
{    
    std::lock_guard<std::mutex> lock(devicesMutex);
    if (!devices.contains(nodeId))
    {
        std::cerr << "Error: Device with ID " << nodeId << " does not exist to increment Transmitting Ack." << std::endl;
        return; // Device does not exist, do not increment
    }
    devices[nodeId]->metrics.incrementTransmittingAck();

}

bool DeviceManager::containsDevice(int id) const
{
    std::lock_guard<std::mutex> lock(devicesMutex);
    return devices.contains(id);
}

std::optional<std::pair<sf::Vector2f, sf::Vector2f>> DeviceManager::getDevicesPosition(int id1, int id2) const
{
        std::lock_guard<std::mutex> lock(devicesMutex);
 auto it1 = devices.find(id1);
    auto it2 = devices.find(id2);

    if (it1 == devices.end() || it2 == devices.end()) {
        std::cerr << "Error: Missing device position(s) for ID(s): "
                  << (it1 == devices.end() ? std::to_string(id1) : "")
                  << ((it1 == devices.end() && it2 == devices.end()) ? " and " : "")
                  << (it2 == devices.end() ? std::to_string(id2) : "") << '\n';
        return std::nullopt;
    }

    return std::make_pair(it1->second->getCenteredPosition(),
                          it2->second->getCenteredPosition());  
    
}

void DeviceManager::clear()
{std::lock_guard<std::mutex> lock(devicesMutex);
    devices.clear();
}
