#include "SimulationController.hpp"
#include "../../Network/Packets/Packets.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"
#include "../../Shared/Libraries/magic_enum.hpp"

void SimulationController::startSimulation(const SimulationConfiguration &config)
{
    std::cout << "Starting simulation..." << std::endl;
    sf::Packet basePacket;

    // Construct launchConfigCommandPacket with required arguments
    // for now, we don't let the user decide on the distance threshold. 
    launchConfigCommandPacket confPacket(1000.0 /*config.getDistanceThreshold()*/, config.getTopologyLines(),static_cast<int>(config.getScenarioType()),config.getMaxSimulationTimeMs());
    basePacket << confPacket;
    TcpServer::instance().transmitPacket(basePacket);
    running.store(true);
}

void SimulationController::pauseSimulation()
{
    std::cout << "Pausing simulation..." << std::endl;
    sf::Packet basePacket;
    pauseCommandPacket pausePacket;
    basePacket << pausePacket;
    TcpServer::instance().transmitPacket(basePacket);
    running.store(false);

}

void SimulationController::resumeSimulation()
{
    // Resume Simulation
    std::cout << "Resuming simulation..." << std::endl;
    sf::Packet basePacket;
    // Construct resumeSimulationCommandPacket with required arguments
    resumeCommandPacket resumePacket;
    basePacket << resumePacket;
    TcpServer::instance().transmitPacket(basePacket);
    running.store(true);
}

void SimulationController::restartSimulation()
{
    std::cout << "Restarting simulation..." << std::endl;
    sf::Packet basePacket;
    restartCommandPacket restartPacket;
    basePacket << restartPacket;
    TcpServer::instance().transmitPacket(basePacket);
    running.store(true);
}

void SimulationController::stopSimulation()
{
    std::cout << "Stopping simulation..." << std::endl;
    sf::Packet basePacket;
    stopCommandPacket stopPacket;
    basePacket << stopPacket;
    TcpServer::instance().transmitPacket(basePacket);
    running.store(false);
}

bool SimulationController::isRunning() const
{
    return running.load();
}
