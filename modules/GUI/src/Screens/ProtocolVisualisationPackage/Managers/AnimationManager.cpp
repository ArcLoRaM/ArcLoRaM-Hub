#include "AnimationManager.hpp"

void AnimationManager::update()
{// Update animations and their life cycle
    {
        std::lock_guard<std::mutex> lock(mutexBroadcastArrow);
        for (auto &animation : broadcastsArrows)
            animation->update();
        //no delete, update only handle the pulsing, no life cycle
    }


    {
        std::lock_guard<std::mutex> lock(mutexDrop);
        for (auto &drop : drops)
            drop->update();
        drops.erase(std::remove_if(drops.begin(), drops.end(),
                                            [](const std::unique_ptr<PacketDrop> &drop)
                                            { return drop->isFinished(); }),
                             drops.end());
    }


    {
        std::lock_guard<std::mutex> lock(mutexReception);
        receptionIcons.erase(std::remove_if(receptionIcons.begin(), receptionIcons.end(),
                                            [](const std::unique_ptr<ReceptionIcon> &icon)
                                            { return icon->isFinished(); }),
                             receptionIcons.end());
    }
}


void AnimationManager::draw(tgui::CanvasSFML::Ptr canvas)
{

    {
        std::lock_guard<std::mutex> lock(mutexBroadcastArrow);
        for (auto &animation : broadcastsArrows)
            animation->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(mutexReception);
        for (auto &icon : receptionIcons)
            icon->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(mutexDrop);
        for (auto &animation : drops)
            animation->draw(canvas);
    }
}



void AnimationManager::addBroadcastArrow(std::unique_ptr<BroadcastArrow> broadcastArrow)
{
        std::lock_guard<std::mutex> lock(mutexBroadcastArrow);
    broadcastsArrows.push_back(std::move(broadcastArrow));
}

void AnimationManager::addReceptionIcon(std::unique_ptr<ReceptionIcon> icon)
{
    std::lock_guard<std::mutex> lock(mutexReception);
    receptionIcons.push_back(std::move(icon));
}

void AnimationManager::addDropAnimation(std::unique_ptr<PacketDrop> drop)
{
    std::lock_guard<std::mutex> lock(mutexDrop);
    drops.push_back(std::move(drop));
}

void AnimationManager::removeBroadcastArrow(int senderId, int receiverId)
{
    std::lock_guard<std::mutex> lock(mutexBroadcastArrow);
    broadcastsArrows.erase(std::remove_if(broadcastsArrows.begin(), broadcastsArrows.end(),
                                [senderId, receiverId](const std::unique_ptr<BroadcastArrow> &broadcastArrow)
                                { return broadcastArrow->getSenderId() == senderId && broadcastArrow->getReceiverId() == receiverId; }),
                 broadcastsArrows.end());
}

void AnimationManager::clear()
{
    std::lock_guard<std::mutex> lockBroadcast(mutexBroadcastArrow);
    std::lock_guard<std::mutex> lockReception(mutexReception);
    std::lock_guard<std::mutex> lockDrop(mutexDrop);

    broadcastsArrows.clear();
    receptionIcons.clear();
    drops.clear();
}