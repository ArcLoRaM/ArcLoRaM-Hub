#pragma once
#include "../INodeFactory.hpp"
// #include other mode factories when available...

#include <memory>
#include <string>
#include <stdexcept>
#include <condition_variable>
#include <mutex>
#include "../../../Connectivity/TCP/Command/CommandDispatcher.hpp"
#include "../../../TDMA/TDMAEnums.hpp"


class FactorySelector {
public:

    static std::unique_ptr<INodeFactory> getFactory(ScenarioType scenario,
                                                    Logger& logger
                                                    );
};



