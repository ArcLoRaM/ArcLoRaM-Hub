#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint> // For uint8_t


 namespace common{

//-----------------------------------------GENERAL PARAMETERS-----------------------------------------
//TODO: put the param below in the conf. file sent.
//TODO: suggested improvement for time duration:
/*
Define your own "tick" duration type
using Tick = std::chrono::duration<int, std::milli>; // 1 tick = 1ms
inline constexpr Tick tickInterval = 10ms;


Now, you can define durations in terms of your virtual tick system:

Tick guardTime = 50ms;
Tick sleepWindow = 1500ms;
*/
constexpr  int tickIntervalForClock_ms=10; //the tick interval for the clock, the lower the more realistic but the more CPU intensive the simulation (minimum 1ms)

//-------------------------------------------
enum class CommunicationMode {
    RRC_Beacon = 1,
    RRC_Downlink = 2,
    RRC_Uplink = 3,
    ENC_Beacon = 11,
    ENC_Downlink = 12,
    ENC_Uplink = 13,
    NotInitialized = 99
};
inline CommunicationMode currentMode = CommunicationMode::NotInitialized;

//-------------------------------------------


enum class PacketType : uint8_t {
    RRC_Beacon = 0x01,
    RRC_Downlink = 0x02,
    RRC_Uplink_Data = 0x03,
    RRC_Uplink_ACK = 0x04
    //... TODO ENC...
};


//-------------------------------------------
//Common Names share between several TDMA (communication ) mode
inline namespace field_names {
    inline constexpr std::string_view type = "type"; //should always be placed in first position in packets
    inline constexpr std::string_view senderGlobalId = "senderGlobalId";
    inline constexpr std::string_view receiverGlobalId = "receiverGlobalId";
    inline constexpr std::string_view globalIDPacket = "globalIDPacket";
    inline constexpr std::string_view localIDPacket = "localIDPacket";
    inline constexpr std::string_view timeStamp = "timeStamp";
    inline constexpr std::string_view costFunction = "costFunction";
    inline constexpr std::string_view hopCount = "hopCount";
    inline constexpr std::string_view payload = "payload";
    inline constexpr std::string_view hashFunction = "hashFunction";
}

//Common Sizes share between several TDMA (communication ) mode
inline namespace field_sizes {
    inline constexpr int type = 1;
    inline constexpr int global_id = 2;
    inline constexpr int global_id_packet = 2;
    inline constexpr int hash = 4;
}


struct FieldInfo {
    size_t offset;
    size_t size;
};

using FieldMap = std::unordered_map<std::string_view, FieldInfo>;
class PacketFormatBuilder {
public:
    PacketFormatBuilder& add(std::string_view name, size_t size) {
        map_[name] = {offset_, size};
        offset_ += size;
        return *this;
    }

    FieldMap build() const {
        return map_;
    }

private:
    FieldMap map_;
    size_t offset_ = 0;
};

//-------------------------------------------

inline namespace rrc_downlink{
    //todo rename those in something that makes more sense (dont use beacon word)
    constexpr  int minimumNbBeaconPackets=2;
    constexpr  int maximumNbBeaconPackets=4;
    constexpr  int nbSlotsPossibleForOneBeacon=10;
    constexpr  int guardTime_ms=50;
    constexpr  int typePacket=0x02;
    constexpr  int timeOnAirFlood_ms=70;


    //For the Time Division Multiple Access Scheme in Seed
    constexpr  unsigned int lengthTransmissionWindow_ms = 1000;
    constexpr  unsigned int lengthSleepingWindow_ms = 1500;
    constexpr  unsigned int nbComWindows =40;

    constexpr  int payloadSizeBytesSize=4;


inline const FieldMap rrc_downlink_fields = PacketFormatBuilder{}
    .add(field_names::type, field_sizes::type)
    .add(field_names::senderGlobalId, field_sizes::global_id)
    .add(field_names::receiverGlobalId, field_sizes::global_id)
    .add(field_names::globalIDPacket, field_sizes::global_id_packet)
    .add(field_names::payload, payloadSizeBytesSize)
    .add(field_names::hashFunction, field_sizes::hash)
    .build();

    //static fields for this mode
    inline constexpr PacketType DownlinkPacketType = PacketType::RRC_Downlink; // Type is 1 byte long in the simulation, 3 bits in real life same for the others !

}

//-------------------------------------------


inline namespace rrc_beacon{
    constexpr  int minimumNbBeaconPackets=2;
    constexpr  int maximumNbBeaconPackets=4;
    constexpr  int nbSlotsPossibleForOneBeacon=10;
    constexpr  int guardTime_ms=50;
    constexpr  int typePacket=0x01;
    constexpr  int timeOnAirBeacon_ms=70;


    constexpr  unsigned int lengthTransmissionWindow_ms = 1000;
    constexpr  unsigned int lengthSleepingWindow_ms = 1500;
    constexpr  unsigned int nbComWindows =40;


    constexpr  int timeStampBytesSize=4;
    constexpr  int costFunctionBytesSize=1;
    constexpr  int hopCountBytesSize=2;

    inline const FieldMap rrc_beacon_fields = PacketFormatBuilder{}
        .add(field_names::type, field_sizes::type)
        .add(field_names::timeStamp, timeStampBytesSize)
        .add(field_names::costFunction, costFunctionBytesSize)
        .add(field_names::hopCount, hopCountBytesSize)
        .add(field_names::globalIDPacket, field_sizes::global_id_packet)
        .add(field_names::senderGlobalId, field_sizes::global_id)
        .add(field_names::hashFunction, field_sizes::hash)
        .build();

    //static fields for this mode
    inline constexpr PacketType BeaconPacketType = PacketType::RRC_Beacon;
}




//-------------------------------------------


inline namespace rrc_uplink{
    constexpr    unsigned int durationSleepWindowMain_ms = 500;      //ms
    constexpr   unsigned int durationDataWindow_ms = 1100; //ms
    constexpr   unsigned int durationSleepWindowSecondary_ms = 500; //ms
    constexpr   unsigned int durationACKWindow_ms = 1100; //ms

    constexpr int numberPacketsReceivedByC3ToStopSimulation =24;

    constexpr  int totalNumberOfSlotsPerModuloNode=5;//Each node will dispose of these slots to potentially transmit
    constexpr  int totalNumberOfSlots=totalNumberOfSlotsPerModuloNode*3; //Modulo three TDMA in the simulation -> multiply by three the number of slots.
    //in all the possible slots for transmission, we will choose a percentage of them to transmit
    inline constexpr float transmissionPercentage = 0.7f;
    inline constexpr  int maxNodeSlots=static_cast<int>(totalNumberOfSlotsPerModuloNode * transmissionPercentage);

    //these variables are adapted for clarity. If we were adopting the ones that duty cycle entails us to take, would be different
    constexpr  int guardTime_ms=80; //ms, added before sending any message to simulate the guard time of the protocol and also prevent race conditions.

    constexpr  int timeOnAirDataPacket_ms=500; //ms
    constexpr  int timeOnAirAckPacket_ms=300; //ms


    //TODO: change the index so it's a variable in the map
    //Put every variables (index + nbBytes) of the fields as consexpr
    //they are used in some function

    //DATA PACKET + ACK PACKET
    constexpr  int localIDPacketBytesSize=2;
    constexpr  int payloadSizeBytesSize=4;

    inline const FieldMap rrc_uplink_data_fields = PacketFormatBuilder{}
        .add(field_names::type, field_sizes::type)
        .add(field_names::senderGlobalId, field_sizes::global_id)
        .add(field_names::receiverGlobalId, field_sizes::global_id)
        .add(field_names::localIDPacket, localIDPacketBytesSize)
        .add(field_names::payload, payloadSizeBytesSize)
        .add(field_names::hashFunction, field_sizes::hash)
        .build();    

        


    inline const FieldMap rrc_uplink_ack_fields = PacketFormatBuilder{}
        .add(field_names::type, field_sizes::type)
        .add(field_names::senderGlobalId, field_sizes::global_id)
        .add(field_names::receiverGlobalId, field_sizes::global_id)
        .add(field_names::localIDPacket, localIDPacketBytesSize)
        .add(field_names::hashFunction, field_sizes::hash)
        .build();    

    inline constexpr PacketType DataPacketType = PacketType::RRC_Uplink_Data;
    inline constexpr PacketType AckPacketType = PacketType::RRC_Uplink_ACK;

}

 }
