#ifndef PACKETDROP_HPP
#define PACKETDROP_HPP

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header

#include <TGUI/Backend/SFML-Graphics.hpp>
class PacketDrop {
public:
    PacketDrop( const sf::Vector2f startPosition, float duration);

    void update(); // No need for deltaTime; internal clock manages timing
    void draw(tgui::CanvasSFML::Ptr canvas);

    bool isFinished() const;

private:
    sf::Texture* m_texture;
    std::optional<sf::Sprite> m_sprite;        // Sprite representing the packet
    sf::Vector2f m_startPosition; // Initial position
    float m_duration;           // Total duration of the animation
    sf::Clock m_clock;          // Internal clock to measure elapsed time

    float calculateYPosition(float t) const;
    float calculateAlpha(float t) const;
};

#endif // PACKETDROP_HPP
