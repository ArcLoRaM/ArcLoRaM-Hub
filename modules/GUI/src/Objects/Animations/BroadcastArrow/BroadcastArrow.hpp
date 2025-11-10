#ifndef BROADCAST_ARROW_HPP
#define BROADCAST_ARROW_HPP

#include <memory>
#include <SFML/Graphics.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include "../Arrow/Arrow.hpp"
#include "../StaticBroadcast/StaticBroadcast.hpp"

class BroadcastArrow {
public:
    BroadcastArrow(const sf::Vector2f& start,
                   const sf::Vector2f& end,
                   int senderId,
                   int receiverId,
                sf::Color arrowColor = sf::Color::Red);

    void update(); // Pass through to subcomponents
    void draw(tgui::CanvasSFML::Ptr canvas) const;

    int getSenderId() const;
    int getReceiverId() const;

private:
    std::unique_ptr<Arrow> arrow;
    std::unique_ptr<StaticBroadcast> broadcast;
    int senderId;
    int receiverId;
};

#endif // BROADCAST_ARROW_HPP
