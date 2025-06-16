
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

#include "Node/Node.hpp"


int main() {

    //todo erase this ?
    sf::TcpListener listener;


//---------------------------------System Initialization---------------------------------
    //Logger
    Logger logger("127.0.0.1",5000);
    logger.start();


    //visualiser configuration
    sf::Packet sysPacketReceiver;
    systemPacket sysPacket(common::distanceThreshold, common::communicationMode);
    sysPacketReceiver<<sysPacket;
    logger.sendTcpPacket(sysPacketReceiver);


    PhyLayer phyLayer(common::distanceThreshold,logger);

//--------------------------------------------------------------Node Provisionning-------------------------------------------------

    



    Seed seed(std::string(common::communicationMode), std::string(common::topology),logger);
    phyLayer.takeOwnership(seed.transferOwnership());    //the seed object memory is released safely


    //Clock

    Clock clock(logger);//the tick interval should not be too small(<=100) otherwise the simulation has unpredicatable behavior (it's not an optimized scheduler I made here)


    phyLayer.registerAllNodeEvents(clock);


  
//---------------------------------Background---------------------------------

    std::atomic<bool> running(true);

    Log startingLog("Starting Simulation...", true);
    logger.logMessage(startingLog);

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
                Log stoppingLog("q pressed, stopping simulation...", true);
                logger.logMessage(stoppingLog);
            }
        }
    }

//---------------------------------End---------------------------------
    clock.stop();
    Log clockstopLog("Scheduler stopped...", true);
    logger.logMessage(clockstopLog);
     Log stoppingLog("transmission loop stopped...", true);
    logger.logMessage(stoppingLog);
    Log stoppingLog2("Simulation Manager stopped...", true);
    logger.logMessage(stoppingLog2);
    Log stoppedLog("Simulation Stopped... Thank you for using ArcLoRaM Simulator", true);
    logger.logMessage(stoppedLog);
    logger.stop();//logger must outlive other objects since it's passed as a reference, otherwise unpredictable behavior

    return 0;
}
