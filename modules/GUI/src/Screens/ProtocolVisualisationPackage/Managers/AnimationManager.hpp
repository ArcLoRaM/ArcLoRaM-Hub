#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <SFML/System/Vector2.hpp>
#include <TGUI/TGUI.hpp> // TGUI header
#include <TGUI/Backend/SFML-Graphics.hpp>
#include "../../../Objects/Animations/BroadcastArrow/BroadcastArrow.hpp"
#include "../../../Objects/Animations/PacketDrop/PacketDrop.hpp"
#include "../../../Objects/Animations/ReceptionIcon/ReceptionIcon.hpp"


class AnimationManager {
public:
    void update();
    void draw(tgui::CanvasSFML::Ptr canvas);

    void addBroadcastArrow(std::unique_ptr<BroadcastArrow> broadcastArrow);
    void removeBroadcastArrow(int senderId, int receiverId);

    void addReceptionIcon(std::unique_ptr<ReceptionIcon> icon);
    void addDropAnimation(std::unique_ptr<PacketDrop> drop);

    void clear();
private:
    std::vector<std::unique_ptr<BroadcastArrow>> broadcastsArrows;
    std::vector<std::unique_ptr<ReceptionIcon>> receptionIcons;
    std::vector<std::unique_ptr<PacketDrop>> drops;

    mutable std::mutex mutexBroadcastArrow;
    mutable std::mutex mutexReception;
    mutable std::mutex mutexDrop;
};
