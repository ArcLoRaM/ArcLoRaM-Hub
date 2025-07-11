#include <iostream>
#include "Shared/RessourceManager/RessourceManager.hpp"
#include "Application/Application.hpp"

int main() {

    try {
        // Load all assets (fonts, textures, etc.)
        ResourceManager::getInstance().loadAll();
        std::cout << "Assets loaded\n";

        // Start the Application
        Application app;
        std::cout << "Application created\n";
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
    
}
