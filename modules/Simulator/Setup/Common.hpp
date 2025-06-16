#ifndef COMMON_HPP
#define COMMON_HPP

#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint> // For uint8_t


 namespace common{




/*
*  Define General Parameters for the Simulation
*   Define the Topology Desired and the Communication Period Mode in the two preprocessor Macros
*   Some parameters specific to the communication mode and the topology are defined in the corresponding sections
*/



/*
 ------------------------------------USE CASES-----------------------------------
There are two major components:
-the topology (position of nodes)
-the Communication Window (Ex: RRC_Beacon, RRC_Downlink, ENC_Beacon), that will dictate the classes of nodes

The seed class will be used to define the topology and the communication window, automatically provisioning the required elements to show the functionnalities of the protocol


Use cases Description:
    For RRC:
            -Topologies:
                -Line
                -Star
                -Mesh

            -Communication Window:
                -RRC_Beacon
                -RRC_Downlink
                -RRC_Uplink
    For ENC:
            -Topologies:
                -Star

            -Communication Window:
                -ENC_Beacon
                -ENC_Downlink
                -ENC_Uplink

If time allows, we will consider an hybrid use case that will combine the two protocols

*/


//-----------------------------------------GENERAL PARAMETERS-----------------------------------------
constexpr const int tickIntervalForClock_ms=20; // The tick interval should not be too low
constexpr const double distanceThreshold=1000; //the distance threshold for the PHY layer
constexpr const bool visualiserConnected=true;//set false if you don't want to display the protocol

//-----------------------------------------Communication Mode and  Topology-----------------------------------------





//-----------------------------------------COMMUNICATION MODE-----------------------------------------
#define COMMUNICATION_PERIOD 3

#define RRC_BEACON 1
#define RRC_DOWNLINK 2
#define RRC_UPLINK 3
#define ENC_BEACON 11   //not implemented
#define ENC_DOWNLINK 12 //not implemented
#define ENC_UPLINK 13   //not implemented


//-----------------------------------------TOPOLOGY-----------------------------------------

#define TOPOLOGY 3
#define LINE 1
#define STAR 2 //not implemented
#define MESH 3
#define MESH_SELF_HEALING 4// A topology that illustrates the self-healing capabilities of the protocol
                         //It supposed to be the second round, so routing is already established
                         //There is a dead node, that suddenly stopped working -> the networks needs to adapt.
                         //only implemented for RRC_Beacon, do not try to run with RRC_UPLINK or Downlink




#if TOPOLOGY == LINE
    constexpr const char* topology = "Line";
#elif TOPOLOGY == MESH
    constexpr const char* topology = "Mesh";
#elif TOPOLOGY == STAR
    constexpr const char* topology = "Star";
#elif TOPOLOGY == MESH_SELF_HEALING
    constexpr const char* topology = "Mesh_Self_Healing";
#else
    #error "Unknown TOPOLOGY mode"
#endif

//This is the new system to transition towards a runtime driven configuration, not at compile time.
enum class CommunicationMode {
    RRC_Beacon = 1,
    RRC_Downlink = 2,
    RRC_Uplink = 3,
    ENC_Beacon = 11,
    ENC_Downlink = 12,
    ENC_Uplink = 13
};

enum class Topology {
    Line = 1,
    Star = 2,
    Mesh = 3,
    Mesh_Self_Healing = 4
};

// Temporary compatibility adapter
constexpr CommunicationMode getCurrentCommunicationMode() {
    return static_cast<CommunicationMode>(COMMUNICATION_PERIOD);
}

constexpr Topology getCurrentTopology() {
    return static_cast<Topology>(TOPOLOGY);
}


#if COMMUNICATION_PERIOD == RRC_DOWNLINK
    constexpr const char* communicationMode = "RRC_Downlink";

    //For the Protocol Stack in Nodes
    constexpr const int minimumNbBeaconPackets=2;
    constexpr const int maximumNbBeaconPackets=4;
    constexpr const int nbSlotsPossibleForOneBeacon=10;
    constexpr const int guardTime=50;
    constexpr const int typePacket=0x02;
    constexpr const int timeOnAirFlood=70;


    //For the Time Division Multiple Access Scheme in Seed
    constexpr  const unsigned int lengthTransmissionWindow = 1000;
    constexpr  const  unsigned int lengthSleepingWindow = 1500;
    constexpr  const  unsigned int nbComWindows =40;

    constexpr const int typeBytesSize=1;
    constexpr const int senderGlobalIdBytesSize=2;
    constexpr const int receiverGlobalIdBytesSize=2;
    constexpr const int globalIDPacketBytesSize=2;
    constexpr const int payloadSizeBytesSize=4;
    constexpr const int hashFunctionBytesSize=4;


    // Format: { "field_name", {start_index, size_in_bytes} }
    inline const std::unordered_map<std::string,std::pair<size_t, size_t>> fieldMap ={

        {"type", {0, typeBytesSize}},             // "type" starts at index 0, 1 byte long
        {"senderGlobalId", {1, senderGlobalIdBytesSize}},       // "senderGlobalId" starts at index 1, 2 bytes long
        {"receiverGlobalId", {3, receiverGlobalIdBytesSize}},    // "receiverGlobalId" starts at index 3, 2 bytes long
        {"globalIDPacket", {5, globalIDPacketBytesSize}},        // "globalIDPacket" starts at index 5, 2 bytes long
        {"payload", {7, payloadSizeBytesSize}},              // "payloadSize" starts at index 7, 4 bytes long
        {"hashFunction", {11, hashFunctionBytesSize}}            // "hashFunction" starts at index 11, 4 bytes long
    };

    //static fields for this mode
    inline  const std::vector<uint8_t> type = {0x02}; // Type is 1 byte long in the simulation, 3 bits in real life


#elif COMMUNICATION_PERIOD == RRC_BEACON

    constexpr const char* communicationMode = "RRC_Beacon";

    //For the Protocol Stack in Nodes
    constexpr const int minimumNbBeaconPackets=2;
    constexpr const int maximumNbBeaconPackets=4;
    constexpr const int nbSlotsPossibleForOneBeacon=10;
    constexpr const int guardTime=50;
    constexpr const int typePacket=0x01;
    constexpr const int timeOnAirBeacon=70;

    //For the Time Division Multiple Access Scheme in Seed
    constexpr  const unsigned int lengthTransmissionWindow = 1000;
    constexpr  const  unsigned int lengthSleepingWindow = 1500;
    constexpr  const  unsigned int nbComWindows =40;




    //For the Packet
    // Define field positions and sizes in the packet

    //TODO: change the index so it's a variable in the map
    //Put every variables (index + nbBytes) of the fields as consexpr
    //they are used in some function
    constexpr const int typeBytesSize=1;
    constexpr const int timeStampBytesSize=4;
    constexpr const int costFunctionBytesSize=1;
    constexpr const int hopCountBytesSize=2;
    constexpr const int globalIDPacketBytesSize=2;
    constexpr const int senderGlobalIdBytesSize=2;
    constexpr const int hashFunctionBytesSize=4;


    // Format: { "field_name", {start_index, size_in_bytes} }
    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> fieldMap = {
            {"type", {0, typeBytesSize}},             // "type" starts at index 0, 1 byte long
            {"timeStamp", {1, timeStampBytesSize}},       // "timeStamp" starts at index 1, 4 bytes long
            {"costFunction", {5, costFunctionBytesSize}},    // "costFunction" starts at index 5, 1 byte long
            {"hopCount", {6, hopCountBytesSize}},        // "hopCount" starts at index 6, 2 bytes long
            {"globalIDPacket", {8, globalIDPacketBytesSize}},  // "globalIDPacket" starts at index 8, 2 bytes long
            {"senderGlobalId", {10, senderGlobalIdBytesSize}}, // "senderGlobalId" starts at index 10, 2 bytes long
            {"hashFunction", {12, hashFunctionBytesSize}},    // "hashFunction" starts at index 12, 4 bytes long
        };

    //static fields for this mode
    inline  const std::vector<uint8_t> type = {0x01}; // Type is 1 byte long in the simulation, 3 bits in real life


#elif COMMUNICATION_PERIOD == RRC_UPLINK


        //if you decrease too much the duration of the Data ack window, nodes are not able to receive the messages (they start sleeping too early...)
    constexpr  char* communicationMode = "RRC_Uplink";
    constexpr    unsigned int durationSleepWindowMain = 500;      //ms
    constexpr   unsigned int durationDataWindow = 1100; //ms
    constexpr   unsigned int durationSleepWindowSecondary = 500; //ms
    constexpr   unsigned int durationACKWindow = 1100; //ms
    constexpr   bool readConfigFromFile = true;

    constexpr int numberPacketsReceivedByC3ToStopSimulation =24;

    constexpr  int totalNumberOfSlotsPerModuloNode=350;//Each node will dispose of these slots to potentially transmit
    constexpr  int totalNumberOfSlots=totalNumberOfSlotsPerModuloNode*3; //Modulo three TDMA in the simulation -> multiply by three the number of slots.
    //in all the possible slots for transmission, we will choose a percentage of them to transmit
    inline constexpr float transmissionPercentage = 0.7f;
    inline constexpr  int maxNodeSlots=static_cast<int>(totalNumberOfSlotsPerModuloNode * transmissionPercentage);

    //these variables are adapted for clarity. If we were adopting the ones that duty cycle entails us to take, would be different
    constexpr  int guardTime=80; //ms, added before sending any message to simulate the guard time of the protocol and also prevent race conditions.

    constexpr  int timeOnAirDataPacket=100; //ms
    constexpr  int timeOnAirAckPacket=60; //ms


    //TODO: change the index so it's a variable in the map
    //Put every variables (index + nbBytes) of the fields as consexpr
    //they are used in some function

    //DATA PACKET + ACK PACKET
    constexpr  int typeBytesSize=1;
    constexpr  int senderGlobalIdBytesSize=2;
    constexpr  int receiverGlobalIdBytesSize=2;
    constexpr  int localIDPacketBytesSize=2;
    constexpr  int payloadSizeBytesSize=4;
    constexpr  int hashFunctionBytesSize=4;

    // Format: { "field_name", {start_index, size_in_bytes} }
    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> dataFieldMap = {
            {"type", {0, typeBytesSize}},             // "type" starts at index 0, 1 byte long
            {"senderGlobalId", {1, senderGlobalIdBytesSize}},       // "senderGlobalId" starts at index 1, 2 bytes long
            {"receiverGlobalId", {3, receiverGlobalIdBytesSize}},    // "receiverGlobalId" starts at index 3, 2 bytes long
            {"localIDPacket", {5, localIDPacketBytesSize}},        // "localIDPacket" starts at index 5, 2 bytes long
            {"payload", {7, payloadSizeBytesSize}},              // "payloadSize" starts at index 7, 4 bytes long
            {"hashFunction", {11, hashFunctionBytesSize}}            // "hashFunction" starts at index 11, 4 bytes long
        };

    inline const std::unordered_map<std::string, std::pair<size_t, size_t>> ackFieldMap = {
            {"type", {0, typeBytesSize}},             // "type" starts at index 0, 1 byte long
            {"senderGlobalId", {1, senderGlobalIdBytesSize}},       // "senderGlobalId" starts at index 1, 2 bytes long
            {"receiverGlobalId", {3, receiverGlobalIdBytesSize}},    // "receiverGlobalId" starts at index 3, 2 bytes long
            {"localIDPacket", {5, localIDPacketBytesSize}},        // "localIDPacket" starts at index 5, 2 bytes long
            {"hashFunction", {7, hashFunctionBytesSize}}            // "hashFunction" starts at index 11, 4 bytes long
        };

    //static fields for this mode
    inline  const std::vector<uint8_t> typeData = {0x03}; // Type is 1 byte long in the simulation, 3 bits in real life
    inline  const std::vector<uint8_t> typeACK = {0x04}; // Type is 1 byte long in the simulation, 3 bits in real life

#elif COMMUNICATION_PERIOD == ENC_BEACON
    const char* communicationMode = "ENC_Beacon";
#else
    #error "Unknown COMMUNICATION_PERIOD mode"
#endif




 }
#endif // COMMON_HPP