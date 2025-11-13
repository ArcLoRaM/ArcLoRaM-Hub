#include "FactorySelector.hpp"
#include "../../../Connectivity/Logger/Logger.hpp"
#include "../../../Setup/Common.hpp"
#include "../SimonV1Factory/SimonV1Factory.hpp"

std::unique_ptr<INodeFactory> FactorySelector::getFactory(ScenarioType scenario,
                                                    Logger& logger                 
                                                    )
{
    switch (scenario) {
        case ScenarioType::SimonV1:
            return std::make_unique<SimonV1Factory>(logger);
        
        // case ScenarioType::SimonV2:
        //     return std::make_unique<SimonV2Factory>(logger);
        
        // case ScenarioType::NinaV1:
        //     return std::make_unique<NinaV1Factory>(logger);
        
        default:
            throw std::invalid_argument("Unsupported scenario type: " + 
                                       std::to_string(static_cast<int>(scenario)));
    }

}