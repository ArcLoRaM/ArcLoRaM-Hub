#include "TcpServer.hpp"

TcpServer::TcpServer() {}

TcpServer::~TcpServer() {
    stop();
}

void TcpServer::start(unsigned short port) {
    if (running.load()) return;

    running.store(true);
    serverThread = std::thread([this, port]() {
        sf::TcpListener listener;
        std::vector<std::unique_ptr<sf::TcpSocket>> clients;

        if (listener.listen(port) != sf::Socket::Status::Done) {
            std::cerr << "Error starting server on port " << port << "\n";
            running.store(false);
            return;
        }

        listener.setBlocking(false);

        while (running.load()) {
            auto client = std::make_unique<sf::TcpSocket>();
            if (listener.accept(*client) == sf::Socket::Status::Done) {
                client->setBlocking(false);
                clients.push_back(std::move(client));
            }

            for (auto& client : clients) {
                sf::Packet packet;
                if (client->receive(packet) == sf::Socket::Status::Done) {
                    processPacket(packet);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        std::cout << "TCP server exiting cleanly\n";
    });
}

void TcpServer::stop() {
    running.store(false);
    if (serverThread.joinable()) {
        serverThread.join();
    }
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
