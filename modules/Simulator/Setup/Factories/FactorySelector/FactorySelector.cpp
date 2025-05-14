#include "FactorySelector.hpp"
#include "../../../Connectivity/Logger/Logger.hpp"
#include "../../../Setup/Common.hpp"


std::unique_ptr<INodeFactory> FactorySelector::getFactory(common::CommunicationMode mode,
                                                          Logger& logger,
                                                          std::condition_variable& dispatchCv,
                                                          std::mutex& dispatchCvMutex,
                                                          uint64_t baseTime)
{
    switch (mode)
    {
    case common::CommunicationMode::RRC_Uplink:
        return std::make_unique<RrcUplinkNodeFactory>(logger, dispatchCv, dispatchCvMutex, baseTime);

    // Later add other factories:
    // case CommunicationMode::RRC_Downlink:
    //     return std::make_unique<RrcDownlinkNodeFactory>(...);

    default:
        throw std::invalid_argument("Unsupported or unimplemented communication mode.");
    }
}