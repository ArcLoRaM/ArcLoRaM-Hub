#ifndef STATICBROADCAST_HPP
#define STATICBROADCAST_HPP

#include <SFML/Graphics.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <vector>

class StaticBroadcast {
public:
    struct Style {
        float radius = 100.f;
        sf::Color baseColor = sf::Color::White;
        float outlineThickness = 1.f;
        bool pulsing = false;

        int ringCount = 6;         // Number of concentric rings
        int maxOpacity = 60;       // Opacity at the center
    };

    StaticBroadcast(const sf::Vector2f& position, const Style& style );
    void update(); // optional pulsing
    void draw(tgui::CanvasSFML::Ptr canvas) const;

private:
    std::vector<sf::CircleShape> rings;
    sf::Vector2f position;

    bool pulsing = false;
    float baseRadius;
    sf::Clock pulseClock;

    void buildRings(const Style& style);
    std::vector<uint8_t> styleAlphaCache;
};

#endif // BROADCAST_ANIMATION_HPP
