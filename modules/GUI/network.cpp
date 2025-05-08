#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>

#include "Packets/Packets.hpp" 
#include "Common/Common.hpp"
#include "VisualiserManager/VisualiserManager.hpp"
#include "PacketDrop/PacketDrop.hpp"


inline void networkThread(VisualiserManager& manager) {
    sf::TcpListener listener;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    // Start listening on port 5000
    if (listener.listen(5000) != sf::Socket::Status::Done) {
        std::cerr << "Error starting server on port 5000\n";
        isRunning = false;
        return;
    }
    std::cout << "Server is running on port 5000...\n";

    listener.setBlocking(false);  // Non-blocking mode

    while (isRunning) {
        // Accept new connections
        auto client = std::make_unique<sf::TcpSocket>();
        if (listener.accept(*client) == sf::Socket::Status::Done) {
            std::cout << "New client connected\n";
            client->setBlocking(false);  // Non-blocking client
            clients.push_back(std::move(client));
        }

        // Receive data from clients
        for (auto& client : clients) {
            sf::Packet packet;
            if (client->receive(packet) == sf::Socket::Status::Done) {
                
                int packetType;
                packet >> packetType; // Read the type first

                switch (packetType) {
                case 0: {
                    systemPacket sp;
                    
                    packet >> sp.distanceThreshold>>sp.mode; // Deserialize the systemPacket
                    std::cout << "Received systemPacket: "
                        << "distanceThreshold=" << sp.distanceThreshold
                        << ", mode=" << sp.mode << std::endl;
                    DISTANCE_THRESHOLD = sp.distanceThreshold*distanceDivider;
                    COMMUNICATION_MODE = sp.mode;
                    std::string message = "Received systemPacket: distanceThreshold=" + std::to_string(sp.distanceThreshold) + ", mode=" + sp.mode;
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    break;
                }
                case 1: {
                    tickPacket tp;
                    packet >> tp.tickNb; // Deserialize the systemPacket
                    // std::cout << "Received tickPacket: "
                    //     << "tickNumber=" << tp.tickNb << std::endl;
                    TICK_NB = tp.tickNb;
                    std::string message = "Received tickPacket: tickNumber=" + std::to_string(tp.tickNb);
                    {
                       // std::lock_guard<std::mutex> lock(logMutex);
                       // logMessages.push_back(message);
                    }
                    break;
                }
                case 2: {
                    stateNodePacket snp;
                    packet >> snp.nodeId >> snp.state; // Deserialize the stateNodePacket
                    // std::cout << "Received stateNodePacket: "
                    //     << "nodeId=" << snp.nodeId
                    //     << ", state=" << snp.state << std::endl;
                    std::string message = "Received stateNodePacket: nodeId=" + std::to_string(snp.nodeId) + ", state=" + snp.state;
                    {
                        //std::lock_guard<std::mutex> lock(logMutex);
                        //logMessages.push_back(message);
                    }
                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        // Update the device state
                        for (auto& device : manager.devices) {
                            if (device->nodeId == snp.nodeId) {
                                device->state = snp.state;
                                device->changePNG(snp.state);
                            }
                        }
                    }
                    //+1 unit of energy if communicate mode (we don't care about C3 so we don't include listen as for the moment it's the only node doing that, when we implement ENC modes, we need to fix this)
                    if(snp.state=="Communicate"){
                        ENERGYEXP++;
                    }
                    break;
                }
                case 3: {
                    //TODO: use sf::Vector2(f?) instead of std::pair 
                    positionPacket pp;
                    packet >>pp.nodeId>> pp.classNode>> pp.coordinates.first>> pp.coordinates.second>>pp.batteryLevel; // Deserialize the positionPacket
                    //make the coordinates received adapted for the screen available to the user
                    pp.coordinates.first+=horizontalOffset;
                    pp.coordinates.second+=verticalOffset;
                    pp.coordinates.first*=distanceDivider;
                    pp.coordinates.second*=distanceDivider;

                    std::cout << "Received positionPacket: "
                        << "nodeId=" << pp.nodeId
                        << ", classNode=" << pp.classNode
                        << "coordinates=(" << pp.coordinates.first
                        << ", " << pp.coordinates.second  <<")"
                        << ", Battery:" <<pp.batteryLevel << std::endl;

                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        // Update the device position
                        //TODO: adapt the magnitude in the simulator to the relative magnitude in the visualisator

                        std::unique_ptr<Device> device = std::make_unique<Device>(pp.nodeId, pp.classNode, pp.coordinates,pp.batteryLevel);
                        manager.addDevice(std::move(device));
                        //for routing
                        manager.addDeviceId(pp.nodeId);
                    }

                    std::string message = "Received positionPacket: nodeId=" + std::to_string(pp.nodeId) + ", coordinates=(" + std::to_string(pp.coordinates.first) + ", " + std::to_string(pp.coordinates.second) + ")";
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    break;
                }

                case 4: {
                    transmitMessagePacket tmp;
                    packet >> tmp.senderId >> tmp.receiverId>>tmp.isACK; // Deserialize the transmitMessagePacket
                    std::cout << "Received transmitMessagePacket: "
                        << "senderId=" << tmp.senderId
                        << ", receiverId=" << tmp.receiverId<<", isACK= "<<tmp.isACK << std::endl;
                    std::string message = "Received transmitMessagePacket: senderId=" + std::to_string(tmp.senderId) + ", receiverId=" + std::to_string(tmp.receiverId);
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        // Create an arrow between the sender and receiver
                        //find senderand receiver Coordinates:
                        sf::Vector2f senderCoordinates;
                        sf::Vector2f receiverCoordinates;
                        bool foundSender = false;
                        bool foundReceiver = false;
                        
                        for(auto& device : manager.devices){
                            if(device->nodeId == tmp.senderId){
                                senderCoordinates =pairToVector2f( device->coordinates);
                                foundSender = true;
                            }
                            if(device->nodeId == tmp.receiverId){
                                receiverCoordinates =pairToVector2f( device->coordinates);
                                foundReceiver = true;
                            }
                        }
                        if(!foundReceiver||!foundSender){
                            {
                                std::lock_guard<std::mutex> lock(logMutex);
                                logMessages.push_back("******Error: Receiver or Sender Not Found for Transmission Animation******");
                            } 
                            std::cout<<"******Error: Receiver or Sender Not Found for Transmission Animation******\n";
                            
                        }
                        else{
                            std::unique_ptr<Arrow> arrow;
                            if(tmp.isACK)   arrow = std::make_unique<Arrow>(senderCoordinates,receiverCoordinates, tmp.senderId,tmp.receiverId,ackArrowColor);
                            else arrow = std::make_unique<Arrow>(senderCoordinates,receiverCoordinates, tmp.senderId,tmp.receiverId, dataArrowColor);
                            manager.addArrow(std::move(arrow));
                        }

                            
                    }
                    ENERGYEXP+=20;
                    break;
                }
                case 5: {
                    receiveMessagePacket rmp;
                    packet >> rmp.senderId >> rmp.receiverId >> rmp.state; // Deserialize the receiveMessagePacket
                    std::cout << "Received receiveMessagePacket: "
                        << "senderId=" << rmp.senderId
                        << ", receiverId=" << rmp.receiverId
                        << ", state=" << rmp.state << std::endl;
                    
                    std::string message = "Received receiveMessagePacket: senderId=" + std::to_string(rmp.senderId) + ", receiverId=" + std::to_string(rmp.receiverId) + ", state=" + rmp.state;
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);

                        // if(COMMUNICATION_MODE=="RRC_Beacon"||COMMUNICATION_MODE=="RRC_Downlink"){
                            //get the location of the sender and receiver
                            sf::Vector2f senderPos;
                            sf::Vector2f receiverPos;
                            bool foundSender=false;
                            bool foundReceiver=false;

                            //should be a function TODO
                        for(auto& device : manager.devices){
                            if(device->nodeId == rmp.senderId){
                                senderPos =pairToVector2f( device->coordinates);
                                foundSender=true;
                            }
                            if(device->nodeId == rmp.receiverId){
                                receiverPos =pairToVector2f( device->coordinates);
                                foundReceiver = true;
                            }
                        }
                        if(!foundReceiver||!foundSender){
                                    {
                                std::lock_guard<std::mutex> lock(logMutex);
                                logMessages.push_back("******Error: Receiver or Sender Not Found for Transmission Animation******");
                            } 
                            std::cout<<"******Error: Receiver or Sender Not Found for Transmission Animation******\n";
                                     
                        }else{
                            std::unique_ptr<ReceptionIcon> icon = std::make_unique<ReceptionIcon>(senderPos,receiverPos,rmp.state);
                            manager.addReceptionIcon(std::move(icon));
                        }

                            //create the receptionIcon and add it to the manager

                        
                        // else{
                        //     //we are in mesh Mode
                        //     manager.changeArrowState(rmp.senderId, rmp.receiverId, rmp.state);

                        // }


                    }

                    break;
                }
                case 6:{
                    routingDecisionPacket rp;
                    packet >> rp.receiverId >> rp.senderId>>rp.newRoute;
                    std::cout << "Received routingDecisionPacket: "
                        << "receiverId=" << rp.receiverId
                        << ", senderId=" << rp.senderId
                        << ", newRoute=" << rp.newRoute << std::endl; 
                    std::string message = "Received routingDecisionPacket: receiverId=" + std::to_string(rp.receiverId) + ", senderId=" + std::to_string(rp.senderId) + ", newRoute=" + std::to_string(rp.newRoute);
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }

                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        if(rp.newRoute){
                            manager.addRouting( rp.receiverId,rp.senderId);
                            //small delay so it looks like the routing is being done upon receiving broadcast
                           // std::this_thread::sleep_for(std::chrono::milliseconds(300));

                        }
                        else{
                            manager.removeRouting(rp.receiverId,rp.senderId);
                        }
                    }

                    break;
                }
                case 7: {
                    broadcastMessagePacket bmp;
                    packet >> bmp.nodeId; // Deserialize the broadcastMessagePacket
                    std::cout << "Received broadcastMessagePacket: "
                        << "nodeId=" << bmp.nodeId << std::endl;
                    std::string message = "Received broadcastMessagePacket: nodeId=" + std::to_string(bmp.nodeId);
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        // Start a broadcast animation
                        sf::Vector2f senderCoordinates;
                        bool foundSender = false;
                        for(auto& device : manager.devices){
                            if(device->nodeId == bmp.nodeId){
                                senderCoordinates =pairToVector2f( device->coordinates);
                                foundSender = true;
                            }
                        }   
                        if(!foundSender){
                            {
                                std::lock_guard<std::mutex> lock(logMutex);
                                logMessages.push_back("******Error: Sender Not Found for Broadcast Animation******");
                            } 
                            std::cout<<"******Error: Sender Not Found for Broadcast Animation******\n";

                        }
                        else{
                            manager.startBroadcast(senderCoordinates, broadcastDuration);
                        }

                    }
                    ENERGYEXP+=20;
                    break;
                }

                case 8:{
                    dropAnimationPacket dap;
                    packet >> dap.nodeId; // Deserialize the dropAnimationPacket
                    std::cout << "Received dropAnimationPacket: "
                        << "nodeId=" << dap.nodeId << std::endl;
                    std::string message = "Received dropAnimationPacket: nodeId=" + std::to_string(dap.nodeId);
                    {
                        std::lock_guard<std::mutex> lock(logMutex);
                        logMessages.push_back(message);
                    }
                    {
                        std::lock_guard<std::mutex> lock(deviceMutex);
                        // Create a drop animation
                        sf::Vector2f senderCoordinates;
                        bool foundSender = false;
                        for(auto& device : manager.devices){
                            if(device->nodeId == dap.nodeId){
                                senderCoordinates =pairToVector2f( device->coordinates);
                                foundSender = true;
                            }
                        }   
                        if(!foundSender){
                            {
                                std::lock_guard<std::mutex> lock(logMutex);
                                logMessages.push_back("******Error: Sender Not Found for Drop Animation******");
                            } 
                            std::cout<<"******Error: Sender Not Found for Drop Animation******\n";

                        }
                        else{
                            std::unique_ptr<PacketDrop> drop = std::make_unique<PacketDrop>(senderCoordinates, 1.6f);
                            manager.addDropAnimation(std::move(drop));
                        }
                    }
                    break;
                }
                case 9:{
                    retransmissionPacket rp;
                    //for now, we simply care about the fact there is retransmission, not the origin, the frequency..,
                    NBRETRANSMISSION++;
                }
                default:
                    std::cerr << "Unknown packet type: " << packetType << std::endl;
                    break;
                }
            }
            }
        }

        // Small delay to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
 }
