#include "SimulationController.hpp"
#include "../../Network/Packets/Packets.hpp"
#include "../../Network/TcpServer/TcpServer.hpp"
#include "../../Shared/Libraries/magic_enum.hpp"

void SimulationController::startSimulation(const TopologyFileState &config)
{
    std::cout << "Starting simulation..." << std::endl;
    sf::Packet basePacket;

    // Construct launchConfigCommandPacket with required arguments
    // for now, we don't let the user decide on the distance threshold. Maybe later we wil
    launchConfigCommandPacket confPacket(1000.0 /*topoFileState.getDistanceThreshold()*/, std::string(magic_enum::enum_name(config.getTDMAMode())), config.getTopologyLines());
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
}

void SimulationController::stopSimulation()
{
}

bool SimulationController::isRunning() const
{
    return running.load();
}
