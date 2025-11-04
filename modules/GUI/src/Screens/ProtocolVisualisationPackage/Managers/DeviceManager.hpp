#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include "../../../Objects/Device/Device.hpp"


class DeviceManager {
public:
    void addDevice(std::unique_ptr<Device> device);
    void update(InputManager& inputManager, tgui::Gui& gui, const tgui::CanvasSFML::Ptr& canvas);
    void draw( std::shared_ptr<tgui::CanvasSFML>& canvas);
    
    void updateDeviceState(int nodeId, DeviceState state);
    std::optional<sf::Vector2f> getDevicePosition(int nodeId) const;

    
    void incrementRetransmission(int nodeId);
    void incrementPacketSent(int nodeId);
    void incrementListeningData(int nodeId);
    void incrementTransmittingData(int nodeId);
    void incrementListeningAck(int nodeId);
    void incrementTransmittingAck(int nodeId);
    std::mutex& getDevicesMutex() const { return devicesMutex; }
    bool containsDevice(int id) const;
    std::optional<std::pair<sf::Vector2f, sf::Vector2f>> getDevicesPosition(int id1, int id2) const;

private:
    mutable std::mutex devicesMutex;
    std::unordered_map<int, std::unique_ptr<Device>> devices;
};
