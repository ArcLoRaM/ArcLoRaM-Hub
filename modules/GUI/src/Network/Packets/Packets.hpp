


#ifndef PACKETS_HPP
#define PACKETS_HPP

#include <string>
#include <optional>
#include <vector>
#include <SFML/Network/Packet.hpp>
#include "../../Screens/ProtocolVisualisationPackage/Metrics/MetricsTypes.hpp"


/*

THIS CLASS SHOULD BE IDENTICAL AS THE ONE PRESENT IN THE GUI.
THINK OF IT AS A SHARED LIBRARY BETWEEN THE TWO APPLICATIONS.

*/


// Base class for packet type identification
class BasePacket {
public:
    int type;
    virtual ~BasePacket() = default; // Virtual destructor for polymorphism
};



//TELEMETRY PACKETS

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

class endTransmissionPacket : public BasePacket {
public:
    int senderId;
    int receiverId;

    endTransmissionPacket(int senderId = 0, int receiverId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const endTransmissionPacket& etp);
    friend sf::Packet& operator>>(sf::Packet& packet, endTransmissionPacket& etp);
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


//THIS IS AN OUTDATED MECHANISM TO STOP SIMULATION from the simulator WHEN CERTAIN CONDItions are met (ex: the gateway received 50 packets)

class stopSimulationPacket : public BasePacket {
    public:
    int nodeId;
    stopSimulationPacket(int nodeId = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const stopSimulationPacket& sp);
    friend sf::Packet& operator>>(sf::Packet& packet, stopSimulationPacket& sp);
};


//METRICS PACKETS

class nodeMetricsPacket : public BasePacket {
public:
    int tickNb;
    int nodeId;
    int totalPacketsSent;
    int totalAcksReceived;

    nodeMetricsPacket(int tickNb = 0, int nodeId = 0, int sent = 0, int acked = 0);
    friend sf::Packet& operator<<(sf::Packet& packet, const nodeMetricsPacket& nmp);
    friend sf::Packet& operator>>(sf::Packet& packet, nodeMetricsPacket& nmp);
};

class latencyRecordsPacket : public BasePacket {
public:
    int tickNb;
    int nodeId;
    std::vector<LatencyRecord> records;

    latencyRecordsPacket(int tickNb = 0, int nodeId = 0, std::vector<LatencyRecord> records = {});
    friend sf::Packet& operator<<(sf::Packet& packet, const latencyRecordsPacket& lrp);
    friend sf::Packet& operator>>(sf::Packet& packet, latencyRecordsPacket& lrp);
};

class energySamplesPacket : public BasePacket {
public:
    int tickNb;
    int nodeId;
    std::vector<std::pair<int, double>> samples;

    energySamplesPacket(int tickNb = 0, int nodeId = 0, std::vector<std::pair<int, double>> samples = {});
    friend sf::Packet& operator<<(sf::Packet& packet, const energySamplesPacket& esp);
    friend sf::Packet& operator>>(sf::Packet& packet, energySamplesPacket& esp);
};


//CONTROL PACKETS

class launchConfigCommandPacket : public BasePacket {

public:

    launchConfigCommandPacket(double threshold,std::string topologyString,int scenarioType);
    
    std::string topologyString;
    double distanceThreshold;
    int scenarioType;
    friend sf::Packet& operator<<(sf::Packet& packet, const launchConfigCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, launchConfigCommandPacket& cmd);
};


class stopCommandPacket : public BasePacket {
public:
    stopCommandPacket() ;
    friend sf::Packet& operator<<(sf::Packet& packet, const stopCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, stopCommandPacket& cmd);
};

class resumeCommandPacket : public BasePacket {
public:
    resumeCommandPacket();
    friend sf::Packet& operator<<(sf::Packet& packet, const resumeCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, resumeCommandPacket& cmd);
};

class pauseCommandPacket : public BasePacket {
public:
    pauseCommandPacket();
    friend sf::Packet& operator<<(sf::Packet& packet, const pauseCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, pauseCommandPacket& cmd);
};

class pingCommandPacket : public BasePacket {
public:
    pingCommandPacket() ;
    
    friend sf::Packet& operator<<(sf::Packet& packet, const pingCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, pingCommandPacket& cmd);
};


class pongPacket : public BasePacket {
public:

    pongPacket();

    friend sf::Packet& operator<<(sf::Packet& packet, const pongPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, pongPacket& cmd);
};

class restartCommandPacket : public BasePacket {
public:
    restartCommandPacket();

    // Optional: could include restart mode or reuse-last-config flag
    
    friend sf::Packet& operator<<(sf::Packet& packet, const restartCommandPacket& cmd);
    friend sf::Packet& operator>>(sf::Packet& packet, restartCommandPacket& cmd);
};




#endif // PACKETS_HPP
