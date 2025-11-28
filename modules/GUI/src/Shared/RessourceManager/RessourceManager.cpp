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

    // Note: TGUI textures are NOT loaded here because they require the TGUI backend
    // to be initialized first (i.e., tgui::Gui must be created before loading TGUI textures).
    // TGUI textures are loaded lazily on first access via getTguiTexture().

    // ... add all other assets here centrally, for the canvas, not for the canvas.
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
    tguiTextures.clear();
}

// Helper function to map enum to file path
std::string ResourceManager::getTguiTexturePath(TguiTextureKey key) {
    switch (key) {
        case TguiTextureKey::PauseButton:
            return "assets/UI/pause.png";
        case TguiTextureKey::PlayButton:
            return "assets/UI/play.png";
        case TguiTextureKey::NormalSpeedButton:
            return "assets/UI/normalSpeed.png";
        case TguiTextureKey::MediumSpeedButton:
            return "assets/UI/mediumSpeed.png";
        case TguiTextureKey::FastSpeedButton:
            return "assets/UI/fastSpeed.png";
        case TguiTextureKey::FastForwardToNextEventButton:
            return "assets/UI/fastForwardToNextEvent.png";
        default:
            throw std::runtime_error("Unknown TguiTextureKey");
    }
}

// Load a TGUI texture from disk
// Note: TGUI internally uses std::shared_ptr for texture data, so if the same
// file path is loaded multiple times across different tgui::Texture objects,
// TGUI's internal TextureManager will deduplicate and share the underlying image data.
// Storing in ResourceManager provides centralized management and type-safe access via enum.
void ResourceManager::loadTguiTexture(TguiTextureKey key) {
    std::string path = getTguiTexturePath(key);
    tgui::Texture texture(path);

    // Check if the texture loaded successfully by checking if it has valid data
    if (texture.getImageSize() == tgui::Vector2u(0, 0)) {
        throw std::runtime_error("Failed to load TGUI texture: " + path);
    }

    tguiTextures.emplace(key, std::move(texture));
}

// Get a TGUI texture by enum key (with lazy loading)
// Returns a const reference - copying tgui::Texture is cheap due to shared_ptr,
// but returning a reference avoids even that small overhead.
// Note: This function implements lazy loading - textures are loaded on first access.
// This is necessary because TGUI textures require the TGUI backend to be initialized
// (i.e., tgui::Gui must exist) before they can be loaded.
const tgui::Texture& ResourceManager::getTguiTexture(TguiTextureKey key) {
    auto it = tguiTextures.find(key);

    // If texture not already loaded, load it now (lazy loading)
    if (it == tguiTextures.end()) {
        loadTguiTexture(key);
        it = tguiTextures.find(key);

        // If still not found after loading, something went wrong
        if (it == tguiTextures.end()) {
            throw std::runtime_error("ResourceManager error: Failed to load TGUI texture.");
        }
    }

    return it->second;
}
