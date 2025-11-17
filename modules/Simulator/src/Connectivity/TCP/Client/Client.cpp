

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
        notConnectedLogArmed.store(true);
        logger.logSystem("Connected to GUI.");
        fireIfSet(onConnectionChanged, true);
        return true;
    }
    return false;
}




bool Client::start() {
    
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


bool Client::transmit(sf::Packet& packet) {
    if (!connected.load()) {
        // Log only on the first failed send after a drop
        if (notConnectedLogArmed.exchange(false)) {
            logger.logError("****** Client is not connected to a server ******");
        }
        return false;
    }
    std::lock_guard<std::mutex> lock(socketMutex);
    sf::Packet temp = packet; // retry unchanged packet
    sf::Socket::Status status;

    do {
        status = socket.send(temp);
        if (status == sf::Socket::Status::Disconnected) {
            logger.logError("Client disconnected during send.");
            connected.store(false);
            socket.disconnect();               // trigger reconnect loop
            notConnectedLogArmed.store(true);   // arm
            if (onConnectionChanged) onConnectionChanged(false);
            return false;
        }
        if (status != sf::Socket::Status::Done && status != sf::Socket::Status::Partial) {
            logger.logError("****** Error sending packet ******");
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



void Client::receiveLoop() {
    while (running.load()) {
        // Reconnect loop (handles the initial connect too)
        while (running.load() && !connected.load()) {
            logger.logSystem("Attempting to connect to server...");
            if (tryConnect()) break;
            logger.logError("****** Error connecting to server. Retrying in 5 seconds... ******");
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
                    connected.store(false);
                    lock.unlock();
                    socket.disconnect();        // ensure clean state for next try
                    notConnectedLogArmed.store(true);
                    fireIfSet(onConnectionChanged, false);
                    break;                      // leave session loop -> reconnect loop
                } else {
                    // Optional: log other errors
                    // logger.logError("Receive error: " + std::to_string(static_cast<int>(status)));
                }
            }
        }
    }
}
