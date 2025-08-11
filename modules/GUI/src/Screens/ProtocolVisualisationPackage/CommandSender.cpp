#include "CommandSender.hpp"
#include "../../Network/Packets/Packets.hpp"

CommandSender::CommandSender(TcpServer& server_) : server(server_) {}

void CommandSender::sendPing() {
    pingCommandPacket ping;
    sf::Packet packet;
    packet << ping;
    server.transmitPacket(packet);
}

void CommandSender::sendStop() {
    stopSimulationPacket stop{-1}; // Add ID if needed
    sf::Packet packet;
    packet << stop;
    server.transmitPacket(packet);
}

void CommandSender::sendRestart() {
    restartCommandPacket cmd;
    sf::Packet packet;
    packet << cmd;
    server.transmitPacket(packet);
}

void CommandSender::sendLaunch(double threshold, const std::string& mode, const std::string& topology) {
    launchConfigCommandPacket launch(threshold, mode, topology);
    sf::Packet packet;
    packet << launch;
    server.transmitPacket(packet);
}

std::string CommandSender::getClientStatus()
{
    return server.getClientStatus();
}
