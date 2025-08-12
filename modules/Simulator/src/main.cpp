
#include "Connectivity/Logger/Logger.hpp"
#include "Connectivity/TCP/Command/CommandManager.hpp"


int main() {

//---------------------------------System Initialization---------------------------------
    //Logger
    Logger logger;
    logger.enableFileOutput("output/log_output.txt");
    logger.enableColorOutput(true); 
    logger.start();

    CommandManager commandManager(logger);


//--------------------------------------------------------------Node Provisionning-------------------------------------------------
    
commandManager.start();

  
//---------------------------------Background---------------------------------
    

//---------------------------------Main---------------------------------

    //we want to stop the simulation when user clicks "q"
    // while (running) {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(300)); // Avoid busy-waiting
    //     if (_kbhit()) { // Check if a key has been pressed
    //         char c = _getch(); // Get the character
    //         if (c == 'q') {
    //             running = false;
    //             logger.logSystem("q pressed,Stopping simulation...");
             
    //         }
    //     }
    // }

//---------------------------------End---------------------------------

    logger.stop();//logger must outlive other objects since it's passed as a reference, otherwise unpredictable behavior

    return 0;
}
