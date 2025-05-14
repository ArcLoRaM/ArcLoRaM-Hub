#include "src/Ui/Button/Button.hpp"
#include "src/Visualisation/VisualiserManager/VisualiserManager.hpp"
#include <iostream>
#include <mutex>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>


//TODO: remove unused includes
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include "src/Network/Packets/Packets.hpp"
#include "src/Shared/Common/Common.hpp"
#include "src/Network/network.cpp"
#include "src/Visualisation/display.cpp"
#include "src/Shared/RessourceManager/RessourceManager.hpp"
#include "src/Application/Application.hpp"


int main() {

    try {
        // Load all assets (fonts, textures, etc.)
        ResourceManager::getInstance().loadAll();

        // Start the Application
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    //  // Load resources: Ressource Manager is alive for the entire program lifecycle, which impacts how we handle memory for SMFL Sprites and Textures (raw pointers, not references)
    // ResourceManager::getInstance().loadAll();
   
    
    // VisualiserManager manager;
    

    // // Create buttons
    // std::unique_ptr<Button> button4 =std::make_unique <Button> (x4coor, y4coor, width4, height4, color4, state4, on, off, iconPath4);
    // manager.addButton(std::move(button4));

    // // Start threads
    // std::thread network(networkThread,std::ref (manager));
    // std::thread display(displayThread, std::ref(manager));


    // // Wait for threads to finish
    // network.join();
    // display.join();

    // return 0;
}
