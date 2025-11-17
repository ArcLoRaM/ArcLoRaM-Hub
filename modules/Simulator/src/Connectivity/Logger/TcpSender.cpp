#include "TcpSender.hpp"
#include <chrono>

void TcpSender::start() {
    tcpStopFlag = false;
    tcpSenderThread = std::thread(&TcpSender::processTcpPackets, this);
}

void TcpSender::stop() {
    {
        std::lock_guard<std::mutex> lock(tcpMutex);
        tcpStopFlag = true;
    }
    tcpCv.notify_all();

    if (tcpSenderThread.joinable()) {
        tcpSenderThread.join();
    }
}

void TcpSender::sendPacket(sf::Packet packet) {
    {
        std::lock_guard<std::mutex> lock(tcpMutex);
        if (packet) {
            tcpQueue.push(packet);
        }
    }
    tcpCv.notify_one();
}

void TcpSender::setClient(Client* clientPtr) {
    client = clientPtr;
}

void TcpSender::processTcpPackets() {
    while (true) {
        sf::Packet packet;

        {
            std::unique_lock<std::mutex> lock(tcpMutex);

            // Exit condition
            if (tcpStopFlag && tcpQueue.empty()) break;

            // Wait for packet or stop signal
            tcpCv.wait(lock, [this] { return !tcpQueue.empty() || tcpStopFlag; });

            if (!tcpQueue.empty()) {
                packet = tcpQueue.front();
            } else {
                continue; // Spurious wakeup or stop flag without packets
            }
        }

        // Attempt to send outside the lock
        if (client && client->transmit(packet)) {
            std::lock_guard<std::mutex> lock(tcpMutex);
            tcpQueue.pop(); // Remove only on successful send
        } else {
            // Retry after a short wait if the visualiser is still connected
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}