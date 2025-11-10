#include "BroadcastArrow.hpp"
#include "../../../Shared/Config.hpp"

BroadcastArrow::BroadcastArrow(const sf::Vector2f& start,
                               const sf::Vector2f& end,
                               int senderId,
                               int receiverId,
                            sf::Color arrowColor)
    : senderId(senderId), receiverId(receiverId)
{
    // Build broadcast background
    StaticBroadcast::Style style;
    style.radius =config::distanceThreshold;
    style.baseColor = sf::Color::White;
    style.maxOpacity = 35;
    style.ringCount = 10;
    style.outlineThickness = 0.2f;
    style.pulsing = true;

    broadcast = std::make_unique<StaticBroadcast>(start, style);
    arrow = std::make_unique<Arrow>(start, end, senderId, receiverId, arrowColor);
}

void BroadcastArrow::update() {
    broadcast->update(); // only needed for pulsing
}

void BroadcastArrow::draw(tgui::CanvasSFML::Ptr canvas) const {
    broadcast->draw(canvas); // background first
    arrow->draw(canvas);     // arrow on top
}

int BroadcastArrow::getSenderId() const {
    return senderId;
}

int BroadcastArrow::getReceiverId() const {
    return receiverId;
}
