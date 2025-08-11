#include "CommandListener.hpp"

CommandListener::CommandListener(unsigned short port_, CommandHandler handler_, Logger& logger_)
    : port(port_), handler(std::move(handler_)), running(false), logger(logger_)
{
    clientSocket.setBlocking(true);
}

CommandListener::~CommandListener() {
    stop();
}

void CommandListener::start() {
    if (running) return;

    if (listener.listen(port) != sf::Socket::Status::Done) {
        logger.logSystem("CommandListener: Failed to bind to port " + std::to_string(port));
        return;
    }

    running = true;
    listenerThread = std::thread(&CommandListener::listenLoop, this);
}

void CommandListener::stop() {
    if (!running) return;

    running = false;
    listener.close(); // force accept to unblock

    if (listenerThread.joinable()) {
        listenerThread.join();
    }

    std::lock_guard lock(socketMutex);
    clientSocket.disconnect();
}

void CommandListener::listenLoop() {

    while (running) {
        {
            std::lock_guard lock(socketMutex);
            if (listener.accept(clientSocket) != sf::Socket::Status::Done) {
                if (running){
                     logger.logSystem("CommandListener: Failed to accept connection. Retrying in 2 seconds...");
                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                }
                   
                continue;
            }
        }

        logger.logSystem("CommandListener: GUI connected.");

        sf::Packet packet;
        while (running && clientSocket.receive(packet) == sf::Socket::Status::Done) {
            handler(packet);
            packet.clear();
        }

        logger.logSystem("CommandListener: GUI disconnected.");

        std::lock_guard lock(socketMutex);
        clientSocket.disconnect();
    }
}
