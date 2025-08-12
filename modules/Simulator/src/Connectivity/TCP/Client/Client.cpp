

#include "Client.hpp"
#include "../../../Setup/Common.hpp"
    #include "../../Logger/Logger.hpp"  

static inline void fireIfSet(std::function<void(bool)>& cb, bool state) {
    if (cb) cb(state);
}


Client::Client(const std::string& serverIp, unsigned short serverPort, Logger& logger)
    : serverIp(serverIp), serverPort(serverPort), logger(logger)
{
    socket.setBlocking(false); // we'll use selector for blocking behavior
}

Client::~Client() {
    stop();
}


bool Client::tryConnect() {
    socket.setBlocking(true);
    auto addr = sf::IpAddress::LocalHost;

    if (socket.connect(addr, serverPort) == sf::Socket::Status::Done) {
        socket.setBlocking(false);
        connected.store(true);
        logger.logSystem("Connected to GUI.");
        fireIfSet(onConnectionChanged, true);
        return true;
    }
    return false;
}

// bool Client::start() {
//     if (!common::visualiserConnected) {
//         logger.logSystem("****** No Visualiser Mode ******");
//         return false;
//     }

//     // Resolve the server IP/name into an IpAddress once
    
//     auto addr = sf::IpAddress::LocalHost;
//     // if (!addr) {
//     //     logger.logSystem("****** Error: could not resolve server address ******");
//     //     return false;
//     // }

//     socket.setBlocking(true);

//     // Retry loop
//     while (running.load() == false) { // make sure we don't start twice
//         logger.logSystem("Attempting to connect to server...");

//         if (socket.connect(addr, serverPort) == sf::Socket::Status::Done) {
//             // Connected!
//             socket.setBlocking(false);
//             connected.store(true);
//             running.store(true);
//             logger.logSystem("Connected to GUI.");

//             receiveThread = std::thread(&Client::receiveLoop, this);
//             return true;
//         }

//         logger.logSystem("****** Error connecting to server. Retrying in 5 seconds... ******");
//         std::this_thread::sleep_for(std::chrono::seconds(5));
//     }

//     return false; // Should only reach here if start() was called while already running
// }


bool Client::start() {
    if (!common::visualiserConnected) {
        logger.logSystem("****** No Visualiser Mode ******");
        return false;
    }
    if (running.exchange(true)) {
        return true; // already running
    }

    receiveThread = std::thread(&Client::receiveLoop, this);
    return true;
}


void Client::stop() {
    if (!running.exchange(false))
        return;

    {
        std::lock_guard<std::mutex> lock(socketMutex);
        if (connected.load()) {
            socket.disconnect();
            connected.store(false);
        }
    }

    if (receiveThread.joinable()) {
        receiveThread.join();
    }

    logger.logSystem("Disconnected from GUI.");
}

// bool Client::transmit( sf::Packet& packet) {
//     if (!connected.load()) {
//         logger.logSystem("****** Client is not connected to a server ******");
//         return false;
//     }

//     std::lock_guard<std::mutex> lock(socketMutex);
//     sf::Socket::Status status;
//     sf::Packet temp = packet; // copy in case we retry

//     do {
//         status = socket.send(temp);
//         if (status == sf::Socket::Status::Disconnected) {
//             logger.logSystem("Client disconnected during send.");
//             connected.store(false);
//             running.store(false);
//             return false;
//         }
//         else if (status != sf::Socket::Status::Done && status != sf::Socket::Status::Partial) {
//             logger.logSystem("****** Error sending packet ******");
//             return false;
//         }
//     } while (status == sf::Socket::Status::Partial);

//     return true;
// }

bool Client::transmit(sf::Packet& packet) {
    if (!connected.load()) {
        logger.logSystem("****** Client is not connected to a server ******");
        return false;
    }

    std::lock_guard<std::mutex> lock(socketMutex);
    sf::Packet temp = packet; // retry unchanged packet
    sf::Socket::Status status;

    do {
        status = socket.send(temp);
        if (status == sf::Socket::Status::Disconnected) {
            logger.logSystem("Client disconnected during send.");
            connected.store(false);
            socket.disconnect();               // trigger reconnect loop
            if (onConnectionChanged) onConnectionChanged(false);
            return false;
        }
        if (status != sf::Socket::Status::Done && status != sf::Socket::Status::Partial) {
            logger.logSystem("****** Error sending packet ******");
            return false;
        }
    } while (status == sf::Socket::Status::Partial);

    return true;
}

bool Client::isConnectedToServer() const {
    return connected.load();
}

void Client::setPacketHandler(PacketHandler handler) {
    packetHandler = std::move(handler);
}

void Client::setConnectionChangedCallback(ConnectionChanged cb) {
    onConnectionChanged = std::move(cb);
}


// void Client::receiveLoop() {
//     while (running.load()) {

//         // If not connected (initially or after a drop), try to connect here
//         while (running.load() && !connected.load()) {
//             logger.logSystem("Attempting to reconnect...");
//             if (tryConnect()) break;
//             logger.logSystem("Reconnect failed. Retrying in 5 seconds...");
//             std::this_thread::sleep_for(reconnectDelay);
//         }
//         if (!running.load()) break; // stop() called

//         // We are connected here: build a fresh selector for this session
//         sf::SocketSelector selector;
//         selector.add(socket);

//         // Inner loop: process this connection until it drops
//         while (running.load() && connected.load()) {
//             if (!selector.wait(sf::milliseconds(100)))
//                 continue; // idle tick

//             std::unique_lock<std::mutex> lock(socketMutex);
//             if (selector.isReady(socket)) {
//                 sf::Packet packet;
//                 sf::Socket::Status status = socket.receive(packet);

//                 if (status == sf::Socket::Status::Done) {
//                     auto cb = packetHandler; // copy under lock
//                     lock.unlock();           // release before user code
//                     if (cb) cb(packet);
//                 } else if (status == sf::Socket::Status::Disconnected) {
//                     // Mark as disconnected and break to the reconnect loop
//                     logger.logSystem("Server disconnected. Entering reconnect loop.");
//                     connected.store(false);
//                     // socket is safe to reuse after disconnect() or connect() call
//                     socket.disconnect(); // ensure clean state
//                     lock.unlock();
//                     break; // exit inner loop -> outer loop will reconnect
//                 } else {
//                     // Other errors; you can log if you want
//                     // logger.logSystem("Receive error: " + std::to_string(static_cast<int>(status)));
//                     // keep the session; selector will keep ticking
//                 }
//             }
//         }
//         // loop continues; if connected==false, outer loop retries
//     }
// }


void Client::receiveLoop() {
    while (running.load()) {
        // Reconnect loop (handles the initial connect too)
        while (running.load() && !connected.load()) {
            logger.logSystem("Attempting to connect to server...");
            if (tryConnect()) break;
            logger.logSystem("****** Error connecting to server. Retrying in 5 seconds... ******");
            std::this_thread::sleep_for(reconnectDelay);
        }
        if (!running.load()) break;

        // Build a fresh selector for this connected session
        sf::SocketSelector selector;
        selector.add(socket);

        // Session loop
        while (running.load() && connected.load()) {
            if (!selector.wait(sf::milliseconds(100)))
                continue;

            std::unique_lock<std::mutex> lock(socketMutex);
            if (selector.isReady(socket)) {
                sf::Packet packet;
                sf::Socket::Status status = socket.receive(packet);

                if (status == sf::Socket::Status::Done) {
                    auto cb = packetHandler; // copy under lock
                    lock.unlock();           // avoid deadlocks
                    if (cb) cb(packet);
                } else if (status == sf::Socket::Status::Disconnected) {
                    logger.logSystem("Server disconnected.");
                    connected.store(false);
                    lock.unlock();
                    socket.disconnect();        // ensure clean state for next try
                    fireIfSet(onConnectionChanged, false);
                    break;                      // leave session loop -> reconnect loop
                } else {
                    // Optional: log other errors
                    // logger.logSystem("Receive error: " + std::to_string(static_cast<int>(status)));
                }
            }
        }
    }
}
