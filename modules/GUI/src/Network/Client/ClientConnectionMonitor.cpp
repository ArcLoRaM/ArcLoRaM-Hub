#include "ClientConnectionMonitor.hpp"
#include "ClientSession.hpp"
#include <iostream>
#include <iostream>

void ClientConnectionMonitor::start()
{
     if (running.exchange(true)) return; // already running
    worker = std::thread(&ClientConnectionMonitor::run, this);
}

void ClientConnectionMonitor::stop()
{
    running.store(false);
    if (worker.joinable())
        worker.join();
}

ClientConnectionMonitor::~ClientConnectionMonitor()
{
stop();
}

void ClientConnectionMonitor::run()
{
        bool lastStatus = false;

    while (running.load()) {
        bool connected = ClientSession::instance().isConnected();
        if (connected != lastStatus && onStatusChanged) {
            onStatusChanged(connected);
            lastStatus = connected;
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}
