#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp> // TGUI header

//Should be a copy of what's in the simulator code
enum class ReceptionState {
    Interference,
    NotListening,
    Received
};


class ReceptionIcon
{

    public:
    ReceptionIcon(const sf::Vector2f& senderPos,const sf::Vector2f& receiverPos, ReceptionState state);

    void draw(tgui::CanvasSFML::Ptr canvas) const;
    bool isFinished() const;

    private:
    sf::Vector2f iconPosition;
    std::optional <sf::Sprite> icon;  // Sprite for the reception state icon
    sf::Texture* interferenceTexture = nullptr;
    sf::Texture* notListeningTexture = nullptr;
    sf::Texture* receivedTexture = nullptr;


    sf::Clock receptionClock;  // Clock for the reception icon duration

     sf::Vector2f getPointOnLine(const sf::Vector2f& senderPos, const sf::Vector2f& receiverPos);
};