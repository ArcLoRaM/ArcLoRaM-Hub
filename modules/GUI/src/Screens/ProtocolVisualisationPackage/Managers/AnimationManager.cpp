#include "AnimationManager.hpp"

void AnimationManager::update()
{// Update animations and their life cycle
    {
        std::lock_guard<std::mutex> lock(mutexBroadcast);
        for (auto &animation : broadcasts)
            animation->update();
        broadcasts.erase(std::remove_if(broadcasts.begin(), broadcasts.end(),
                                                 [](const std::unique_ptr<BroadcastAnimation> &animation)
                                                 { return animation->isFinished(); }),
                                  broadcasts.end());
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
        std::lock_guard<std::mutex> lock(mutexArrow);
        for (auto &arrow : arrows)
            arrow->update();
        arrows.erase(std::remove_if(arrows.begin(), arrows.end(),
                                    [](const std::unique_ptr<Arrow> &arrow)
                                    { return arrow->isFinished() && arrow->isReceptionFinished(); }),
                     arrows.end());
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
{{
        std::lock_guard<std::mutex> lock(mutexArrow);
        for (auto &arrow : arrows)
            arrow->draw(canvas);
    }

    {
        std::lock_guard<std::mutex> lock(mutexBroadcast);
        for (auto &animation : broadcasts)
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

void AnimationManager::addBroadcast(const sf::Vector2f &startPosition, float duration)
{   
     std::lock_guard<std::mutex> lock(mutexBroadcast);
    broadcasts.push_back(std::make_unique<BroadcastAnimation>(startPosition, duration));

}

void AnimationManager::addArrow(std::unique_ptr<Arrow> arrow)
{
        std::lock_guard<std::mutex> lock(mutexArrow);
    arrows.push_back(std::move(arrow));
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

void AnimationManager::clear()
{
    std::lock_guard<std::mutex> lockBroadcast(mutexBroadcast);
    std::lock_guard<std::mutex> lockArrow(mutexArrow);
    std::lock_guard<std::mutex> lockReception(mutexReception);
    std::lock_guard<std::mutex> lockDrop(mutexDrop);

    broadcasts.clear();
    arrows.clear();
    receptionIcons.clear();
    drops.clear();
}

// void AnimationManager::changeArrowState(int senderId, int receiverId, const std::string &state)
// {    std::lock_guard<std::mutex> lock(mutexArrow);
//     if (state != "interference" && state != "notListening" && state != "received")
//     {
//         throw std::runtime_error("Error: state not recognized");
//     }
//     for (auto &arrow : arrows)
//     {
//         if (arrow->SenderId == senderId && arrow->ReceiverId == receiverId)
//         {
//             arrow->receptionState = state;
//         }
//     }
// }
