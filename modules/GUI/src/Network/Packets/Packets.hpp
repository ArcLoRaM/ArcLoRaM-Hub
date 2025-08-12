


#ifndef PACKETS_HPP
#define PACKETS_HPP

#include <string>
#include <optional>
#include <SFML/Network/Packet.hpp>


// Base class for packet type identification
class BasePacket {
public:
    int type;
    virtual ~BasePacket() = default; // Virtual destructor for polymorphism
};



//TELEMETRY PACKETS



// Packet class declarations
class systemPacket : public BasePacket {
public:
    double distanceThreshold;
    std::string mode;

    systemPacket(double distanceThreshold = 100, std::string mode = "error");
    friend sf::Packet& operator<<(sf::Packet& packet, const systemPacket& sp);
    friend sf::Packet& operator>>(sf::Packet& packet, systemPacket& sp);
};

class tickPacket : public BasePacket {
public:
    int tickNb;

    tickPacket(int tickNb = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const tickPacket& tp);
    friend sf::Packet& operator>>(sf::Packet& packet, tickPacket& tp);
};



class stateNodePacket : public BasePacket {
public:
    int nodeId;
    std::string state;
    std::optional<bool> isCommunicatingAck; // used for energy expenditure calculation 


    stateNodePacket(int nodeId = 0, std::string state = "error");
    friend sf::Packet& operator<<(sf::Packet& packet, const stateNodePacket& snp);
    friend sf::Packet& operator>>(sf::Packet& packet, stateNodePacket& snp);
};
sf::Packet& operator>>(sf::Packet& packet, stateNodePacket& snp);
sf::Packet& operator<<(sf::Packet& packet, const stateNodePacket& snp);



//should be renamed initialNodePacket
class positionPacket : public BasePacket {
public:
    int nodeId;
    std::pair<int, int> coordinates;
    int classNode;
    double batteryLevel;
    int hopCount;
    positionPacket(int id=0, int classNode =0,std::pair<int, int> coordinates = { 0, 0 }, double batteryLevel=0.0, int hopCount = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const positionPacket& pp);
    friend sf::Packet& operator>>(sf::Packet& packet, positionPacket& pp);
};

class transmitMessagePacket : public BasePacket {
public:
    int senderId;
    int receiverId;
    bool isACK;

    transmitMessagePacket(int senderId = 0, int receiverId = 0,bool isACK=false);
    friend sf::Packet& operator<<(sf::Packet& packet, const transmitMessagePacket& tmp);
    friend sf::Packet& operator>>(sf::Packet& packet, transmitMessagePacket& tmp);
};

class receiveMessagePacket : public BasePacket {
public:
    int senderId;
    int receiverId;
    std::string state;

    receiveMessagePacket(int senderId = 0, int receiverId = 0, std::string state = "error");
    friend sf::Packet& operator<<(sf::Packet& packet, const receiveMessagePacket& rmp);
    friend sf::Packet& operator>>(sf::Packet& packet, receiveMessagePacket& rmp);
};

class routingDecisionPacket : public BasePacket {
public:
    int receiverId;
    int senderId;
    bool newRoute;

    routingDecisionPacket(int receiverId = 0, int senderId = 0, bool newRoute = false);
    friend sf::Packet& operator<<(sf::Packet& packet, const routingDecisionPacket& rdp);
    friend sf::Packet& operator>>(sf::Packet& packet, routingDecisionPacket& rdp);
};

class broadcastMessagePacket : public BasePacket {
public:
    int nodeId;

    broadcastMessagePacket(int nodeId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const broadcastMessagePacket& bmp);
    friend sf::Packet& operator>>(sf::Packet& packet, broadcastMessagePacket& bmp);
};


class dropAnimationPacket : public BasePacket {
public:
    int nodeId;

    dropAnimationPacket(int nodeId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const dropAnimationPacket& bmp);
    friend sf::Packet& operator>>(sf::Packet& packet, dropAnimationPacket& bmp);
};

class retransmissionPacket : public BasePacket {
    public:
    int nodeId;
    retransmissionPacket(int nodeId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const retransmissionPacket& rp);
    friend sf::Packet& operator>>(sf::Packet& packet, retransmissionPacket& rp);
};

class stopSimulationPacket : public BasePacket {
    public:
    int nodeId;
    stopSimulationPacket(int nodeId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const stopSimulationPacket& sp);
    friend sf::Packet& operator>>(sf::Packet& packet, stopSimulationPacket& sp);
};




//CONTROL PACKETS

class launchConfigCommandPacket : public BasePacket {

public:
    double distanceThreshold;
    std::string communicationMode;
    std::string topology;
    launchConfigCommandPacket(double distanceThreshold = 100, std::string communicationMode = "error", std::string topology = "error");
    // Future use — optional now
    std::vector<std::string> configLines; // Raw node lines from .simcfg file

    friend sf::Packet& operator<<(sf::Packet& packet, const launchConfigCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, launchConfigCommandPacket& cmd);
};

class pingCommandPacket : public BasePacket {
public:
    pingCommandPacket() { type = 103; }
    
    friend sf::Packet& operator<<(sf::Packet& packet, const pingCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, pingCommandPacket& cmd);
};


class pongPacket : public BasePacket {
public:

    pongPacket() 
         { type = 104; }

    friend sf::Packet& operator<<(sf::Packet& packet, const pongPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, pongPacket& cmd);
};

class restartCommandPacket : public BasePacket {
public:
    restartCommandPacket() { type = 105; }

    // Optional: could include restart mode or reuse-last-config flag
    
    friend sf::Packet& operator<<(sf::Packet& packet, const restartCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, restartCommandPacket& cmd);
};




#endif // PACKETS_HPP
