#include "TcpServer.hpp"
#include <mutex>

TcpServer::TcpServer() {


}

TcpServer::~TcpServer() {
    stop();
}
void TcpServer::start(unsigned short port) {
    if (running.load()) return;
    running.store(true);

    serverThread = std::thread([this, port]() {
        sf::TcpListener listener;
        if (listener.listen(port) != sf::Socket::Status::Done) {
            std::cerr << "Error starting server on port " << port << "\n";
            running.store(false);
            return;
        }

        // With a selector, we don't need non-blocking mode or manual sleeps.
        // Just register sockets and wait.
        sf::SocketSelector selector;
        selector.add(listener);

        while (running.load()) {
            // Optional: use a timeout if you want periodic wakeups (e.g., 100ms)
            if (!selector.wait(sf::milliseconds(100))) {
                continue; // timeout: loop again, check running flag
            }

            // Check for new connection
            if (selector.isReady(listener)) {
                std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
                if (listener.accept(*newClient) == sf::Socket::Status::Done) {
                    // Only allow localhost
                    if (newClient->getRemoteAddress() == sf::IpAddress::LocalHost) {
                        std::lock_guard<std::mutex> lock(clientMutex);
                        if (!client) {
                            client = std::move(newClient);
                            selector.add(*client);
                            std::cout << "Client from localhost connected.\n";
                        } else {
                            std::cout << "Rejected extra localhost connection.\n";
                            newClient->disconnect();
                        }
                    } else {
                        std::cout << "Rejected client from " 
                                  << newClient->getRemoteAddress()->toString() << "\n";
                        newClient->disconnect();
                    }
                }
            }

            // Data ready from the client?
            std::unique_lock<std::mutex> lock(clientMutex);
            if (client && selector.isReady(*client)) {
                sf::Packet packet;
                sf::Socket::Status st = client->receive(packet);
                if (st == sf::Socket::Status::Done) {
                    lock.unlock(); // avoid holding the mutex during user code
                    processPacket(packet);
                } else if (st == sf::Socket::Status::Disconnected) {
                    // Cleanly remove the client from selector and reset
                    selector.remove(*client);
                    client.reset();
                    // std::cout << "Client disconnected\n";
                } else {
                    // Error or NotReady (rare with selector); handle/log as needed
                    // std::cerr << "Receive error: " << static_cast<int>(st) << "\n";
                }
            } else {
                lock.unlock();
            }
        }

        // Cleanup on exit
        std::lock_guard<std::mutex> lock(clientMutex);
        if (client) {
            selector.remove(*client);
            client->disconnect();
            client.reset();
        }
        // std::cout << "TCP server exiting cleanly\n";
    });
}


void TcpServer::stop() {
    running.store(false);
    if (serverThread.joinable()) {
        serverThread.join();
    }
}

std::string TcpServer::getClientStatus()
{
    if (!client) {
        return "No client connected.";
    }

    std::ostringstream oss;
    
    
    oss << "Client: " << client->getRemoteAddress()->toString() << " is connected.\n";
    
    
    return oss.str();
}

void TcpServer::setPacketHandler(PacketHandler handler) {
    packetHandler = std::move(handler);
}

void TcpServer::processPacket(sf::Packet& packet) {
    if (packetHandler) {
        packetHandler(packet);
    } else {
        std::cerr << "Warning: Packet received but no handler set.\n";
    }
}

void TcpServer::transmitPacket(sf::Packet& packet) {
    std::lock_guard<std::mutex> lock(clientMutex);

    if (!client) {
        std::cerr << "No client connected to transmit packet.\n";
        return;
    }

    // Retry the exact same packet if we get Partial.
    sf::Packet toSend = packet; // keep the original intact
    sf::Socket::Status status;

    do {
        status = client->send(toSend);

        if (status == sf::Socket::Status::Disconnected) {
            std::cerr << "Client disconnected during send.\n";
            client.reset();
            return;
        }

        if (status != sf::Socket::Status::Done &&
            status != sf::Socket::Status::Partial) {
            std::cerr << "Failed to transmit packet to client. Socket error code: "
                      << static_cast<int>(status) << "\n";
            return;
        }
        // If Partial, loop and resend the same packet.
    } while (status == sf::Socket::Status::Partial);
}

