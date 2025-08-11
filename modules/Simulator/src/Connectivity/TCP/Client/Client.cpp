

#include "Client.hpp"
#include "../../../Setup/Common.hpp"
    #include "../../Logger/Logger.hpp"  


// Client::Client(const std::string& serverIp, unsigned short serverPort,  Logger& logger)
//     : isConnected(false), logger(logger) {
//     if(!common::visualiserConnected){
//         std::cout << "****** No Visualiser Mode ******\n";
//         return;
//     }
//     // Attempt to connect to the server
//     auto localAddress = sf::IpAddress::getLocalAddress();
//     if (!localAddress.has_value() || socket.connect(localAddress.value(), serverPort) != sf::Socket::Status::Done) {
//         std::cerr << "******Error connecting to server******\n";
//         return;
//     }

//     logger.logSystem("Connected to GUI");
//     isConnected = true;
// }

// Client::~Client() {
//     if (isConnected) {
//         socket.disconnect();
//         logger.logSystem("Disconnected from GUI.");
//     }
// }

// bool Client::transmit(sf::Packet& packet) {
//     if (!isConnected) {
//         logger.logSystem("******Client is not connected to a server.******");
//         return false;
//     }

//     const void* data = packet.getData();
//     std::size_t totalSize = packet.getDataSize();
//     std::size_t sentBytes = 0;

//     while (sentBytes < totalSize) {
//         std::size_t justSent = 0;
//         sf::Socket::Status status = socket.send(
//             static_cast<const char*>(data) + sentBytes,
//             totalSize - sentBytes,
//             justSent
//         );

//         if (status == sf::Socket::Status::Done || status == sf::Socket::Status::Partial) {
//             sentBytes += justSent;
//         } else {
//             logger.logSystem("******Error sending packet.******");
//             return false;
//         }
//     }

//     return true;
// }


// bool Client::isConnectedToServer() const {
//     return isConnected;
// }


#include "Client.hpp"
#include "../../../Setup/Common.hpp"

Client::Client(const std::string& serverIp, unsigned short serverPort, Logger& logger)
    : serverIp(serverIp), serverPort(serverPort), logger(logger)
{
    socket.setBlocking(false); // we'll use selector for blocking behavior
}

Client::~Client() {
    stop();
}

bool Client::start() {
    if (!common::visualiserConnected) {
        logger.logSystem("****** No Visualiser Mode ******");
        return false;
    }

    // Resolve the server IP/name into an IpAddress once
    
    auto addr = sf::IpAddress::LocalHost;
    // if (!addr) {
    //     logger.logSystem("****** Error: could not resolve server address ******");
    //     return false;
    // }

    socket.setBlocking(true);

    // Retry loop
    while (running.load() == false) { // make sure we don't start twice
        logger.logSystem("Attempting to connect to server...");

        if (socket.connect(addr, serverPort) == sf::Socket::Status::Done) {
            // Connected!
            socket.setBlocking(false);
            connected.store(true);
            running.store(true);
            logger.logSystem("Connected to GUI.");

            receiveThread = std::thread(&Client::receiveLoop, this);
            return true;
        }

        logger.logSystem("****** Error connecting to server. Retrying in 5 seconds... ******");
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return false; // Should only reach here if start() was called while already running
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
bool Client::transmit( sf::Packet& packet) {
    if (!connected.load()) {
        logger.logSystem("****** Client is not connected to a server ******");
        return false;
    }

    std::lock_guard<std::mutex> lock(socketMutex);
    sf::Socket::Status status;
    sf::Packet temp = packet; // copy in case we retry

    do {
        status = socket.send(temp);
        if (status == sf::Socket::Status::Disconnected) {
            logger.logSystem("Client disconnected during send.");
            connected.store(false);
            running.store(false);
            return false;
        }
        else if (status != sf::Socket::Status::Done && status != sf::Socket::Status::Partial) {
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

void Client::receiveLoop() {
    sf::SocketSelector selector;
    selector.add(socket);

    while (running.load()) {
        if (!selector.wait(sf::milliseconds(100)))
            continue; // timeout — check running flag

        std::unique_lock<std::mutex> lock(socketMutex);
        if (selector.isReady(socket)) {
            sf::Packet packet;
            sf::Socket::Status status = socket.receive(packet);

            if (status == sf::Socket::Status::Done) {
                //thread safety trick to avoid deadlocks when calling user supplied callback
                auto cb = packetHandler;  // copy under lock
                lock.unlock();            // release before calling user code
                if (cb) cb(packet);
            } else if (status == sf::Socket::Status::Disconnected) {
                logger.logSystem("Server disconnected.");
                connected.store(false);
                running.store(false);
                break;
            } else  {
                logger.logSystem("Receive error: " + std::to_string(static_cast<int>(status)));
            }
        }
    }
}
