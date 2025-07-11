
#include <thread>
#include "PhyLayer/PhyLayer.hpp"
#include "Connectivity/Logger/Logger.hpp"
#include <atomic>
#include <list>
#include <conio.h> // For _kbhit() and _getch()
#include "Node/C3/C3_Node.hpp"
#include "Node/C2/C2_Node.hpp"
#include "Node/C1/C1_Node.hpp"
#include "Node/Clock/Clock.hpp"
#include "Setup/Seed/Seed.hpp"
#include "Setup/Common.hpp"
#include "Connectivity/TCP/Telemetry/Client.hpp"
#include "Node/Node.hpp"
#include "Connectivity/TCP/Control/CommandDispatcher.hpp"
#include "Connectivity/TCP/Control/CommandListener.hpp"

int main() {

//---------------------------------System Initialization---------------------------------
    //Logger
    Logger logger;
    Client tcpClient("127.0.0.1", 5000);

    logger.setTcpClient(&tcpClient);
    logger.enableFileOutput("output/log_output.txt");
    logger.enableColorOutput(true); 
    logger.start();


    CommandDispatcher dispatcher(logger);
    CommandListener listener(6000, [&dispatcher](sf::Packet& p) {
        dispatcher.onCommand(p);
    },logger);

    listener.start();


    logger.logSystem("Waiting for launch config from GUI...");
    std::optional<LaunchConfig> configOpt;
    while (!(configOpt = dispatcher.getPendingLaunchConfig())) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Once available:
    LaunchConfig config = *configOpt;


    //visualiser configuration
    sf::Packet sysPacketReceiver;
    systemPacket sysPacket(config.distanceThreshold, config.communicationMode);
    sysPacketReceiver<<sysPacket;
    logger.sendTcpPacket(sysPacketReceiver);


    PhyLayer phyLayer(config.distanceThreshold,logger);

//--------------------------------------------------------------Node Provisionning-------------------------------------------------

    



    Seed seed(std::string(config.communicationMode), std::string(config.topology),logger);
    phyLayer.takeOwnership(seed.transferOwnership());    //the seed object memory is released safely

    

    //Clock (aka as the scheduler)

    Clock clock(logger);


    phyLayer.registerAllNodeEvents(clock);


  
//---------------------------------Background---------------------------------

    std::atomic<bool> running(true);

    logger.logSystem("Simulation started");

    // The main thread
    clock.start();
    

//---------------------------------Main---------------------------------

    //we want to stop the simulation when user clicks "q"
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Avoid busy-waiting
        if (_kbhit()) { // Check if a key has been pressed
            char c = _getch(); // Get the character
            if (c == 'q') {
                running = false;
                logger.logSystem("q pressed,Stopping simulation...");
             
            }
        }
    }

//---------------------------------End---------------------------------
    clock.stop();
    logger.logSystem("Clock stopped...");

    logger.logSystem("Simulation stopped... Thank you for using ArcLoRaM Framework!");

    logger.stop();//logger must outlive other objects since it's passed as a reference, otherwise unpredictable behavior

    return 0;
}
