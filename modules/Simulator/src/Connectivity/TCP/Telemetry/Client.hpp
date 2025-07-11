#ifndef CLIENT_HPP
#define CLIENT_HPP


#include <SFML/Network.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

class Client {
    ////////////////////////////////////////////////////////////
    /// \brief Sends packets to a server using TCP
    ///
    /// \param serverIp   IP address of the server
    /// \param serverPort Port number of the server
    ///
    ////////////////////////////////////////////////////////////
public:
    // Constructor: Connect to the server
    Client(const std::string& serverIp, unsigned short serverPort);

    // Destructor: Disconnect from the server
    ~Client();

    // Transmit a packet to the server
    bool transmit(sf::Packet& packet);

    // Check if the client is connected to the server
    bool isConnectedToServer() const;

private:
    sf::TcpSocket socket;
    bool isConnected;
};

#endif // CLIENT_HPP
