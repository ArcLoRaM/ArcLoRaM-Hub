#pragma once

#include <atomic>
#include "../States/TopologyFileState.hpp"

class SimulationController {
public:
    void startSimulation(const TopologyFileState& config);
    void pauseSimulation();
    void resumeSimulation();
    void restartSimulation();
    void stopSimulation();

    bool isRunning() const;

private:
    std::atomic<bool> running = false;
};
