#include "TcpServer.hpp"
#include <mutex>
#include "ClientSession.hpp"
#include "../Packets/Packets.hpp"


void TcpServer::start(unsigned short port) {
    if (running.load()) return;
    running.store(true);

    serverThread = std::thread([this, port]()
                               {
                                   sf::TcpListener listener;
                                   if (listener.listen(port) != sf::Socket::Status::Done)
                                   {
                                       std::cerr << "Error starting server on port " << port << "\n";
                                       running.store(false);
                                       return;
                                   }

                                   // With a selector, we don't need non-blocking mode or manual sleeps.
                                   // Just register sockets and wait.
                                   sf::SocketSelector selector;
                                   selector.add(listener);

                                   // while (running.load()) {
                                   //     // Optional: use a timeout if you want periodic wakeups (e.g., 100ms)
                                   //     if (!selector.wait(sf::milliseconds(600))) {
                                   //         tickHeartbeat();
                                   //         continue; // timeout: loop again, check running flag
                                   //     }

                                   //     // Check for new connection
                                   //     if (selector.isReady(listener)) {
                                   //         std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
                                   //         if (listener.accept(*newClient) == sf::Socket::Status::Done) {
                                   //             // Only allow localhost
                                   //             if (newClient->getRemoteAddress() == sf::IpAddress::LocalHost) {
                                   //                 std::lock_guard<std::mutex> lock(clientMutex);
                                   //                 if (!client) {
                                   //                     client = std::move(newClient);
                                   //                     selector.add(*client);
                                   //                     auto ip = client->getRemoteAddress();
                                   //                     ClientSession::instance().setConnected(true, ip);
                                   //                     std::cout << "Client from localhost connected.\n";
                                   //                     lastPing = std::chrono::steady_clock::now();
                                   //                     lastPong = std::chrono::steady_clock::now();
                                   //                 } else {
                                   //                     std::cout << "Rejected extra localhost connection.\n";
                                   //                     newClient->disconnect();
                                   //                 }
                                   //             } else {
                                   //                 std::cout << "Rejected client from "
                                   //                           << newClient->getRemoteAddress()->toString() << "\n";
                                   //                 newClient->disconnect();
                                   //             }
                                   //         }
                                   //     }

                                   //     // Data ready from the client?
                                   //     std::unique_lock<std::mutex> lock(clientMutex);
                                   //     if (client && selector.isReady(*client)) {
                                   //         sf::Packet packet;
                                   //         sf::Socket::Status st = client->receive(packet);
                                   //         if (st == sf::Socket::Status::Done) {
                                   //             lock.unlock(); // avoid holding the mutex during user code
                                   //             processPacket(packet);
                                   //         } else if (st == sf::Socket::Status::Disconnected) {
                                   //             // Cleanly remove the client from selector and reset
                                   //             selector.remove(*client);
                                   //             client.reset();
                                   //             // std::cout << "Client disconnected\n";
                                   //         } else {
                                   //             // Error or NotReady (rare with selector); handle/log as needed
                                   //             // std::cerr << "Receive error: " << static_cast<int>(st) << "\n";
                                   //         }
                                   //     } else {
                                   //         lock.unlock();
                                   //     }
                                   //     tickHeartbeat();
                                   // }

                                   while (running.load())
                                   {
                                       const bool ready = selector.wait(sf::milliseconds(100));

                                       // 1) Accept (no clientMutex here)
                                       if (ready && selector.isReady(listener))
                                       {
                                           std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
                                           if (listener.accept(*newClient) == sf::Socket::Status::Done)
                                           {
                                               if (newClient->getRemoteAddress() == sf::IpAddress::LocalHost)
                                               {
                                                   std::lock_guard<std::mutex> lock(clientMutex);
                                                   if (!client)
                                                   {
                                                       client = std::move(newClient);
                                                       selector.add(*client);
                                                       auto ip = client->getRemoteAddress();
                                                       ClientSession::instance().setConnected(true, ip);
                                                       std::cout << "Client from localhost connected.\n";
                                                       lastPing = std::chrono::steady_clock::now();
                                                       lastPong = std::chrono::steady_clock::now();
                                                   }
                                                   else
                                                   {
                                                                                                          std::cout << "Rejected extra localhost connection.\n";

                                                       newClient->disconnect();
                                                   }
                                               }
                                               else
                                               {
                                                   newClient->disconnect();
                                               }
                                           }
                                       }

                                       // 2) Receive (lock only inside this scope)
                                       {
                                           std::unique_lock<std::mutex> lock(clientMutex);
                                           if (ready && client && selector.isReady(*client))
                                           {
                                               sf::Packet packet;
                                               auto st = client->receive(packet);
                                               if (st == sf::Socket::Status::Done)
                                               {
                                                   lock.unlock(); // release BEFORE user code
                                                   processPacket(packet);
                                               }
                                               else if (st == sf::Socket::Status::Disconnected)
                                               {
                                                   selector.remove(*client);
                                                   client.reset();
                                                   lock.unlock();
                                                   ClientSession::instance().setConnected(false, std::nullopt);
                                               } // else: error/NotReady; just fall through with lock released by scope
                                           }
                                           // lock released automatically at end of scope
                                       }

                                       // 3) Heartbeat — MUST be outside any lock scope
                                       tickHeartbeat();
                                   }

                                   // Cleanup on exit
                                   std::lock_guard<std::mutex> lock(clientMutex);
                                   if (client)
                                   {
                                       selector.remove(*client);
                                       client->disconnect();
                                       client.reset();
                                   }
                                   // std::cout << "TCP server exiting cleanly\n";
                               });
}

void TcpServer::stop()
{
    std::cout << "Stopping TCP server...\n";
    running.store(false);
    if (serverThread.joinable())
    {
        serverThread.join();
    }
    ClientSession::instance().setConnected(false, std::nullopt);

    std::cout << "TCP server stopped.\n";
}

void TcpServer::setPacketHandler(PacketHandler handler)
{
    packetHandler = std::move(handler);
}

void TcpServer::tickHeartbeat()
{
    if(!client)return;

    // Send ping (no clientMutex held here)
    if (std::chrono::steady_clock::now() - lastPing >= pingInterval)
    {
        sf::Packet pingPacket;
        pingCommandPacket pingPckt; // <- no vexing parse
        pingPacket << pingPckt;
        transmitPacket(pingPacket); // locks internally
        lastPing = std::chrono::steady_clock::now();
    }

    // Timeout check (brief lock)
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        if (client && std::chrono::steady_clock::now() - lastPong > pongTimeout)
        {
            std::cerr << "Heartbeat timeout: dropping client\n";
            client->disconnect(); // this will wake selector
            client.reset();
            ClientSession::instance().setConnected(false, std::nullopt);
        }
    }
}

void TcpServer::processPacket(sf::Packet &packet)
{
    if (packetHandler)
    {
        packetHandler(packet);
    }
    else
    {
        std::cerr << "Warning: Packet received but no handler set.\n";
    }
}

void TcpServer::transmitPacket(sf::Packet &packet)
{
    std::lock_guard<std::mutex> lock(clientMutex);

    if (!client)
    {
        std::cerr << "No client connected to transmit packet.\n";
        return;
    }

    // Retry the exact same packet if we get Partial.
    sf::Packet toSend = packet; // keep the original intact
    sf::Socket::Status status;

    do
    {
        status = client->send(toSend);

        if (status == sf::Socket::Status::Disconnected)
        {
            std::cerr << "Client disconnected during send.\n";
            client.reset();
            return;
        }

        if (status != sf::Socket::Status::Done &&
            status != sf::Socket::Status::Partial)
        {
            std::cerr << "Failed to transmit packet to client. Socket error code: "
                      << static_cast<int>(status) << "\n";
            return;
        }
        // If Partial, loop and resend the same packet.
    } while (status == sf::Socket::Status::Partial);
}

void TcpServer::updateLastPong()
{
    lastPong = std::chrono::steady_clock::now();
}
