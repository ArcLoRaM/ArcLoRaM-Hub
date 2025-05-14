#pragma once
#include "../INodeFactory.hpp"
#include "../RrcUplinkNodeFactory/RrcUplinkNodeFactory.hpp"
// #include other mode factories when available...

#include <memory>
#include <string>
#include <stdexcept>
#include <condition_variable>
#include <mutex>




class FactorySelector {
public:
    static std::unique_ptr<INodeFactory> getFactory(common::CommunicationMode mode,
                                                    Logger& logger,
                                                    std::condition_variable& dispatchCv,
                                                    std::mutex& dispatchCvMutex,
                                                    uint64_t baseTime);
};