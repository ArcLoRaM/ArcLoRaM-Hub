#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <stdexcept>

class ResourceManager {
public:
    static ResourceManager& getInstance();

    void loadAll(); // To preload everything

    sf::Font& getFont(const std::string& key);
    sf::Texture& getTexture(const std::string& key);

    void clear();

private:
    ResourceManager() = default;
    void loadFont(const std::string& key, const std::string& path);
    void loadTexture(const std::string& key, const std::string& path);

    std::unordered_map<std::string, sf::Font> fonts;
    std::unordered_map<std::string, sf::Texture> textures;
};
