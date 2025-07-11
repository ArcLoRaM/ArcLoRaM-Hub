
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
#include "Connectivity/TCP/Client.hpp"
#include "Node/Node.hpp"


int main() {




//---------------------------------System Initialization---------------------------------
    //Logger
    Client tcpClient("127.0.0.1", 5000);
    Logger logger;
    logger.setTcpClient(&tcpClient);
    logger.enableFileOutput("output/log_output.txt");
    logger.enableColorOutput(true); 
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
