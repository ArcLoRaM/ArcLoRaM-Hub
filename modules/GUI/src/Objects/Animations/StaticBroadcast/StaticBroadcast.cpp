#include "StaticBroadcast.hpp"

#include <cmath>

StaticBroadcast::StaticBroadcast(const sf::Vector2f& position, const Style& style)
    : position(position), pulsing(style.pulsing), baseRadius(style.radius)
{
    buildRings(style);
}

void StaticBroadcast::buildRings(const Style& style) {
    rings.clear();
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
    }
}

void StaticBroadcast::update() {
    if (!pulsing) return;

    float t = pulseClock.getElapsedTime().asSeconds();
    float scale = 1.f + 0.05f * std::sin(t * 3.f); // 3 Hz pulse

    for (auto& ring : rings) {
        float r = ring.getRadius();
        float newR = baseRadius * scale * (r / baseRadius);
        ring.setRadius(newR);
        ring.setOrigin({newR, newR}); // Re-center
    }
}

void StaticBroadcast::draw(tgui::CanvasSFML::Ptr canvas) const {
    for (const auto& ring : rings) {
        canvas->draw(ring);
    }
}
