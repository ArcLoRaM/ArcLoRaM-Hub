#pragma once

#include <thread>
#include <atomic>
#include <functional>

class ClientConnectionMonitor {
public:
    void start();
    void stop();
    ~ClientConnectionMonitor();

    std::function<void(bool connected)> onStatusChanged;

private:
    void run();
    std::atomic<bool> running{false};
    std::thread worker;
};
