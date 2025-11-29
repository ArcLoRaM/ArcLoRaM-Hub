#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

// Enum for SFML texture keys (type-safe resource access)
enum class SfmlTextureKey {
    // Device icons (DeviceClass + DeviceState combinations)
    C3_Sleep,
    C3_Listen,
    C3_Transmit,
    C3_Communicate,
    C2_Sleep,
    C2_Listen,
    C2_Transmit,
    C2_Communicate,
    // Reception icons
    Reception_Interference,
    Reception_NotListening,
    Reception_AllGood,
    // Packet animation
    Packet_Letter,
    // UI elements
    Rooting_Button
};

// Hash function for SfmlTextureKey to use as unordered_map key
struct SfmlTextureKeyHash {
    std::size_t operator()(SfmlTextureKey key) const {
        return static_cast<std::size_t>(key);
    }
};

// Enum for TGUI texture keys (type-safe resource access)
enum class TguiTextureKey {
    PauseButton,
    PlayButton,
    NormalSpeedButton,
    MediumSpeedButton,
    FastSpeedButton,
    FastForwardToNextEventButton,
    GatewayIcon,
    RelayIcon,
    EndNodeIcon
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

    // SFML texture management with enum-based keys
    sf::Texture& getTexture(SfmlTextureKey key);

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
    void loadTexture(SfmlTextureKey key);

    // Helpers to convert enums to file paths
    std::string getSfmlTexturePath(SfmlTextureKey key);
    std::string getTguiTexturePath(TguiTextureKey key);

    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<SfmlTextureKey, sf::Texture, SfmlTextureKeyHash> textures;

    // TGUI textures stored separately with enum keys
    std::unordered_map<TguiTextureKey, tgui::Texture, TguiTextureKeyHash> tguiTextures;
};
