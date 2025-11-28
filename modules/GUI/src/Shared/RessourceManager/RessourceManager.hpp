#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

// Enum for TGUI texture keys (type-safe resource access)
enum class TguiTextureKey {
    PauseButton,
    PlayButton,
    NormalSpeedButton,
    MediumSpeedButton,
    FastSpeedButton,
    FastForwardToNextEventButton
};

// Hash function for TguiTextureKey to use as unordered_map key
struct TguiTextureKeyHash {
    std::size_t operator()(TguiTextureKey key) const {
        return static_cast<std::size_t>(key);
    }
};

class ResourceManager {
public:
    static ResourceManager& getInstance();

    void loadAll(); // To preload everything

    sf::Font& getFont(const std::string& key);
    sf::Texture& getTexture(const std::string& key);

    // TGUI texture management
    // Note: TGUI uses std::shared_ptr internally for textures, so copying is cheap
    // and TGUI automatically deduplicates textures loaded from the same file path.
    // We store them here for centralized management and type-safe enum-based access.
    void loadTguiTexture(TguiTextureKey key);
    const tgui::Texture& getTguiTexture(TguiTextureKey key);

    void clear();

private:
    ResourceManager() = default;
    void loadFont(const std::string& key, const std::string& path);
    void loadTexture(const std::string& key, const std::string& path);

    // Helper to convert enum to file path
    std::string getTguiTexturePath(TguiTextureKey key);

    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<std::string, sf::Texture> textures;

    // TGUI textures stored separately with enum keys
    std::unordered_map<TguiTextureKey, tgui::Texture, TguiTextureKeyHash> tguiTextures;
};
