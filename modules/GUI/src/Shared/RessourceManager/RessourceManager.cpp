#include "RessourceManager.hpp"

ResourceManager& ResourceManager::getInstance() {
    static ResourceManager instance;
    return instance;
}

void ResourceManager::loadFont(const std::string& key, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path))
        throw std::runtime_error("Failed to load font: " + path);

    fonts.emplace(key, std::move(font));
}

void ResourceManager::loadTexture(const std::string& key, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path))
        throw std::runtime_error("Failed to load texture: " + path);

    textures.emplace(key, std::move(texture));
}

void ResourceManager::loadAll() {
    //Todo. arial still needed?
    loadFont("Arial", "assets/arial.ttf");
    loadTexture("C3_Sleep", "assets/Icons/C3_Sleep.png");
    loadTexture("C2_Sleep", "assets/Icons/C2_Sleep.png");
    loadTexture("C3_Transmit", "assets/Icons/C3_Transmit.png");
    loadTexture("C2_Transmit", "assets/Icons/C2_Transmit.png");
    loadTexture("C3_Listen", "assets/Icons/C3_Listen.png");
    loadTexture("C2_Listen", "assets/Icons/C2_Listen.png");
    loadTexture("C3_Communicate", "assets/Icons/C3_Communicate.png");
    loadTexture("C2_Communicate", "assets/Icons/C2_Communicate.png");


    loadTexture("Reception_Interference", "assets/Reception/interference.png");
    loadTexture("Reception_NotListening", "assets/Reception/notListening.png");
    loadTexture("Reception_AllGood", "assets/Reception/allGood.png");
    loadTexture("Packet_Letter", "assets/PacketDrop/letter.png");
    loadTexture("Rooting_Button", "assets/Icons/routing.png");
    // ... add all other assets here centrally
}


//todo: still needed?
sf::Font& ResourceManager::getFont(const std::string& key) {
    auto it = fonts.find(key);
    if (it == fonts.end()) {
        throw std::runtime_error("ResourceManager error: Font with key '" + key + "' was not loaded. "
                                 "Ensure it is loaded in ResourceManager::loadAll().");
    }
    return it->second;
}


sf::Texture& ResourceManager::getTexture(const std::string& key) {
    auto it = textures.find(key);
    if (it == textures.end()) {
        throw std::runtime_error("Texture with key '" + key + "' was not loaded");
    }
    return it->second;
}

void ResourceManager::clear() {
    fonts.clear();
    textures.clear();
}
