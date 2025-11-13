#pragma once
#include "../INodeFactory.hpp"
#include "../../../Connectivity/Logger/Logger.hpp"
#include "../../../TDMA/TdmaScheduleBlueprint/TdmaScheduleBlueprint.hpp"
#include <memory>

class SimonV1Factory : public INodeFactory {
public:
    SimonV1Factory(Logger& logger);

    std::shared_ptr<C3_Node> createC3Node(int id, std::pair<int, int> coordinates) override;
    std::shared_ptr<C2_Node> createC2Node(int id, std::pair<int, int> coordinates, std::optional<int> nextHop = std::nullopt, std::optional<int> hopCount = std::nullopt) override;
    
    std::shared_ptr<C1_Node> createC1Node(int id, std::pair<int, int> coordinates) override;
    
    // Get the configured blueprint (for Seed to pass to PhyLayer if needed)
    std::shared_ptr<IScheduleBlueprint> getBlueprint() const { return blueprint; }

private:
    Logger& logger;
    std::shared_ptr<TdmaScheduleBlueprint> blueprint;
    
    void configureBlueprint();
};