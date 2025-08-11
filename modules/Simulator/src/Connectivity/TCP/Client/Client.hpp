    #ifndef CLIENT_HPP
    #define CLIENT_HPP


    #include <SFML/Network.hpp>
    #include <iostream>
    #include <memory>
    #include <string>
    #include <thread>
    #include <chrono>

    
    class Logger;  // forward declare


    // class Client {
    //     ////////////////////////////////////////////////////////////
    //     /// \brief Sends packets to a server using TCP
    //     ///
    //     /// \param serverIp   IP address of the server
    //     /// \param serverPort Port number of the server
    //     ///
    //     ////////////////////////////////////////////////////////////
    // public:
    //     // Constructor: Connect to the server
    //     Client(const std::string& serverIp, unsigned short serverPort, Logger& logger);

    //     // Destructor: Disconnect from the server
    //     ~Client();

    //     // Transmit a packet to the server
    //     bool transmit(sf::Packet& packet);

    //     // Check if the client is connected to the server
    //     bool isConnectedToServer() const;

    // private:
    //     sf::TcpSocket socket;
    //     bool isConnected;
    //     Logger& logger; 
    // };





#
#include <functional>



class Client {
public:
    using PacketHandler = std::function<void(sf::Packet&)>;

    Client(const std::string& serverIp, unsigned short serverPort, Logger& logger);
    ~Client();

    bool start();
    void stop();

    bool transmit( sf::Packet& packet);
    bool isConnectedToServer() const;

    void setPacketHandler(PacketHandler handler);

private:
    void receiveLoop();

    sf::TcpSocket socket;
    std::thread receiveThread;
    std::mutex socketMutex;
    std::atomic<bool> running{false};
    std::atomic<bool> connected{false};

    std::string serverIp;
    unsigned short serverPort;
    Logger& logger;

    PacketHandler packetHandler;
};


    #endif // CLIENT_HPP
