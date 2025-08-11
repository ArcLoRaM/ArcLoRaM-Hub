#pragma once

#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <queue>
#include <optional>
#include <iostream>
#include "../../Logger/Logger.hpp"  

/*

Create the server that receives packet and dispatches commands to the CommandDispatcher.


*/


class CommandListener {
public:
    using CommandHandler = std::function<void(sf::Packet&)>;

    CommandListener(unsigned short port, CommandHandler handler, Logger& logger);
    ~CommandListener();

    void start();
    void stop();

private:
    void listenLoop();

    sf::TcpListener listener;
    sf::TcpSocket clientSocket;
    std::thread listenerThread;
    std::atomic<bool> running;

    unsigned short port;
    CommandHandler handler;
    Logger& logger;

    std::mutex socketMutex;
};
