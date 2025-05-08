#include "PacketDrop.hpp"

PacketDrop::PacketDrop( const sf::Vector2f startPosition, float duration)
    :  m_startPosition(startPosition), m_duration(duration) {

    // Load a texture for the packet icon
    sf::Texture texture;
    if (!texture.loadFromFile("assets/PacketDrop/letter.png")) {
        throw std::runtime_error("Failed to load packet texture");
    }    // Set the sprite to be fully opaque at the start
    m_sprite = std::make_optional<sf::Sprite>(texture);
    m_sprite->setScale(sf::Vector2f(0.3f, 0.3f)); // Scale to 50% of its original size

    sf::Color color = m_sprite->getColor();
    color.a = 255;
    m_sprite->setColor(color);
    m_sprite->setPosition(startPosition);

}

void PacketDrop::update() {
    float elapsedTime = m_clock.getElapsedTime().asSeconds();

    if (elapsedTime < m_duration) {
        // Normalize elapsed time (0 to 1)
        float t = elapsedTime / m_duration;

        // Calculate new position based on the inverted U-curve
        float x = m_startPosition.x + t * 50.0f; // Slight leftward movement
        float y = calculateYPosition(t);

        m_sprite->setPosition(sf::Vector2f(x, y));

        // Update transparency
        sf::Color color = m_sprite->getColor();
        color.a = static_cast<uint8_t>(calculateAlpha(t) * 255);
        m_sprite->setColor(color);
    }
}

void PacketDrop::draw(sf::RenderWindow& window) {
    if (!isFinished()) {
        window.draw(*m_sprite);
    }
}

bool PacketDrop::isFinished() const {
    return m_clock.getElapsedTime().asSeconds() >= m_duration;
}

float PacketDrop::calculateYPosition(float t) const {
    // Quadratic inverted U-curve: starts high, goes up, then curves down
    return m_startPosition.y + (4 * (t - 0.5f) * (t - 0.5f) - 1) * 50.0f; // Scale vertical range with 50.0f
}

float PacketDrop::calculateAlpha(float t) const {
    // Linearly fade alpha from 1 (fully opaque) to 0 (fully transparent)
    return 1.0f - t;
}
