#pragma once

#include <atomic>
#include "../States/SimulationConfiguration.hpp"

class SimulationController {
public:
    void startSimulation(const SimulationConfiguration& config);
    void pauseSimulation();
    void resumeSimulation();
    void restartSimulation();
    void stopSimulation();

    bool isRunning() const;

private:
    std::atomic<bool> running = false;
};
