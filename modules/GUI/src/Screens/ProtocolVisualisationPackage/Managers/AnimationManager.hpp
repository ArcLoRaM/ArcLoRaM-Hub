#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <SFML/System/Vector2.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../../Objects/Animations/Arrow/Arrow.hpp"
#include "../../../Objects/Animations/BroadcastAnimation/BroadcastAnimation.hpp"
#include "../../../Objects/Animations/PacketDrop/PacketDrop.hpp"
#include "../../../Objects/Animations/ReceptionIcon/ReceptionIcon.hpp"


class AnimationManager {
public:
    void update();
    void draw(tgui::CanvasSFML::Ptr canvas);

    void addBroadcast(const sf::Vector2f& startPosition, float duration);
    void addArrow(std::unique_ptr<Arrow> arrow);
    void addReceptionIcon(std::unique_ptr<ReceptionIcon> icon);
    void addDropAnimation(std::unique_ptr<PacketDrop> drop);
    // void changeArrowState(int senderId, int receiverId, const std::string& state);

private:
    std::vector<std::unique_ptr<BroadcastAnimation>> broadcasts;
    std::vector<std::unique_ptr<Arrow>> arrows;
    std::vector<std::unique_ptr<ReceptionIcon>> receptionIcons;
    std::vector<std::unique_ptr<PacketDrop>> drops;

    mutable std::mutex mutexBroadcast;
    mutable std::mutex mutexArrow;
    mutable std::mutex mutexReception;
    mutable std::mutex mutexDrop;
};
