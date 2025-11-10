#include "StaticBroadcast.hpp"

#include <cmath>

StaticBroadcast::StaticBroadcast(const sf::Vector2f& position, const Style& style)
    : position(position), pulsing(style.pulsing), baseRadius(style.radius)
{
    buildRings(style);
}

void StaticBroadcast::buildRings(const Style& style) {
    rings.clear();

    
  // Sanitize input
    if (style.ringCount <= 0 || style.radius <= 0.f) {
        // Fallback to a single ring
        sf::CircleShape ring(style.radius > 0.f ? style.radius : 50.f);
        sf::Color ringColor = style.baseColor;
        ringColor.a = static_cast<uint8_t>(style.maxOpacity);

        ring.setFillColor(ringColor);
        ring.setOutlineColor(style.baseColor);
        ring.setOutlineThickness(style.outlineThickness);
        ring.setOrigin({ring.getRadius(), ring.getRadius()});
        ring.setPosition(position);
        rings.push_back(ring);
        return;
    }

    float step = style.radius / static_cast<float>(style.ringCount);

    for (int i = 0; i < style.ringCount; ++i) {
        float ringRadius = step * (i + 1);
        int alpha = static_cast<int>(style.maxOpacity * (1.0f - static_cast<float>(i) / style.ringCount));

        sf::Color ringColor = style.baseColor;
        ringColor.a = static_cast<uint8_t>(alpha);

        sf::CircleShape ring(ringRadius);
        ring.setFillColor(ringColor);
        ring.setOutlineColor(style.baseColor);
        ring.setOutlineThickness(style.outlineThickness);
        ring.setOrigin({ringRadius, ringRadius});
        ring.setPosition(position);

        rings.push_back(ring);
        styleAlphaCache.push_back(static_cast<uint8_t>(alpha));

    }

}

void StaticBroadcast::update() {
    if (!pulsing) return;
    float t = pulseClock.getElapsedTime().asSeconds();
    float pulseFactor = 1.f + 0.2f * std::sin(t * 3.f); // ±20% pulse, 3 Hz

    for (size_t i = 0; i < rings.size(); ++i) {
        float baseAlpha = static_cast<float>(styleAlphaCache[i]); // stored base alpha
        float newAlpha = std::clamp(baseAlpha * pulseFactor, 0.f, 255.f);

        sf::Color color = rings[i].getFillColor();
        color.a = static_cast<uint8_t>(newAlpha);
        rings[i].setFillColor(color);
    }
}

void StaticBroadcast::draw(tgui::CanvasSFML::Ptr canvas) const {
    for (const auto& ring : rings) {
        canvas->draw(ring);
    }
}
