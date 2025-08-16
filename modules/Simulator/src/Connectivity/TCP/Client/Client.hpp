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


//todo: this could be a singleton but is it worth the effort?

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

  using ConnectionChanged = std::function<void(bool up)>; // true=connected, false=disconnected
    void setConnectionChangedCallback(ConnectionChanged cb);  

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

    bool tryConnect();               // one attempt (blocking), returns true on success
    std::chrono::seconds reconnectDelay{5};

    ConnectionChanged onConnectionChanged;    // user-provided callback


    std::atomic<bool> notConnectedLogArmed{true}; // log "not connected" only once per drop

};


    #endif // CLIENT_HPP
