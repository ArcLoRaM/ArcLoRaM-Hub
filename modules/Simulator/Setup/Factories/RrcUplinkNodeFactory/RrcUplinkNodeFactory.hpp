#pragma once
#include "../INodeFactory.hpp"
#include "../../../Connectivity/Logger/Logger.hpp"
#include <condition_variable>
#include <mutex>


class RrcUplinkNodeFactory : public INodeFactory {
public:
    RrcUplinkNodeFactory(Logger& logger, std::condition_variable& dispatchCv, std::mutex& dispatchCvMutex, uint64_t baseTime);

    std::shared_ptr<C3_Node> createC3Node(int id, std::pair<int, int> coordinates) override;
    std::shared_ptr<C2_Node> createC2Node(int id, std::pair<int, int> coordinates, int nextHop, int hopCount) override;
    std::shared_ptr<C1_Node> createC1Node(int id, std::pair<int, int> coordinates) override;  // explicitly unsupported

private:
    Logger& logger;
    std::condition_variable& dispatchCv;
    std::mutex& dispatchCvMutex;
    uint64_t baseTime;
};
