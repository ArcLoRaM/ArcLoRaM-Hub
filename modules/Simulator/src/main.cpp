
#include "Connectivity/Logger/Logger.hpp"
#include "Connectivity/TCP/Command/CommandManager.hpp"
#include <conio.h> // For _kbhit() and _getch()

int main() {

//---------------------------------System Initialization---------------------------------
    //Logger
    Logger logger;
    logger.enableFileOutput("output/log_output.txt");
    logger.enableColorOutput(true); 
    logger.start();

    CommandManager commandManager(logger);
    
    std::thread commandThread([&commandManager]() {
        commandManager.start();
    });
    commandThread.detach();
    
//---------------------------------Main---------------------------------
    bool running = true;
    // we want to stop the simulation when user clicks "q"
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

    //Todo: signal the commandManager to stop gracefully 

//---------------------------------End---------------------------------

    logger.stop();//logger must outlive other objects since it's passed as a reference, otherwise unpredictable behavior

    return 0;
}
