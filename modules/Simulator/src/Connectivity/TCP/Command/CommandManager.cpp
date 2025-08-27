#include "CommandManager.hpp"
#include "Setup/Common.hpp"
#include "Setup/Seed/Seed.hpp"
#include "Node/Node.hpp"
#include "../Packets/Packets.hpp"

CommandManager::CommandManager(Logger &logger_)
    : logger(logger_),
      tcpClient("127.0.0.1", 5000,logger_),
      dispatcher(logger),
      running(false)
{

    logger.setTcpClient(&tcpClient);
    tcpClient.setPacketHandler([this](sf::Packet& p) {
        dispatcher.onCommand(p);
    });

    dispatcher.setStopCallback([this]()
                               { 
                             logger.logSystem("Stop callback triggered.");

                                // this->stopSimulation();
                             });

    dispatcher.setPingCallback([this]()
                               {
                                //    logger.logSystem("Ping callback triggered, sending pong");
                                    sf::Packet pongBasePacket;
                                    pongPacket pongPck;
                                    pongBasePacket << pongPck;
                                    logger.sendTcpPacket(pongBasePacket);
                               });

    dispatcher.setRestartCallback([this]() {
            logger.logSystem("Restart callback triggered.");
            this->stopSimulation();
    });



    tcpClient.setConnectionChangedCallback([this](bool up){
    if (!up) {
        logger.logSystem("Connection lost — stopping simulation.");
        this->stopSimulation();
    } else {
        logger.logSystem("Connection established.");
        // If you want to auto-resume or re-request config, do it here.
    }
});

}

CommandManager::~CommandManager()
{

    stop();

}

void CommandManager::start()
{

    tcpClient.start();

    logger.logSystem("Waiting for launch config from GUI...");
    waitForLaunchConfig();
}

void CommandManager::waitForLaunchConfig()
{
    std::optional<LaunchConfig> configOpt;
    while (!(configOpt = dispatcher.getPendingLaunchConfig()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    launchSimulation(*configOpt);
}

void CommandManager::launchSimulation(const LaunchConfig &config)
{
    logger.logSystem("Configuration received. Launching simulation...");

    // Notify GUI of system parameters, information is sent back as a security check TODO
    sf::Packet sysPacket;
    systemPacket sys(config.distanceThreshold, config.communicationMode);
    sysPacket << sys;
    logger.sendTcpPacket(sysPacket);


    
    if (config.communicationMode == "RRC_Uplink")  common::currentMode = common::CommunicationMode::RRC_Uplink;
    else if (config.communicationMode == "RRC_Downlink")  common::currentMode = common::CommunicationMode::RRC_Downlink;
    else if (config.communicationMode == "RRC_Beacon")  common::currentMode = common::CommunicationMode::RRC_Beacon;
    else if (config.communicationMode == "ENC_Uplink")  common::currentMode = common::CommunicationMode::ENC_Uplink;
    else if (config.communicationMode == "ENC_Downlink")  common::currentMode = common::CommunicationMode::ENC_Downlink;
    else if (config.communicationMode == "ENC_Beacon")  common::currentMode = common::CommunicationMode::ENC_Beacon;
    else throw std::invalid_argument("Unknown MODE specified: " + config.communicationMode);


    // Build simulation, TODO: add path loss model here?
    phyLayer = std::make_unique<PhyLayer>(config.distanceThreshold, logger);
    Seed seed(config.topologyLines, logger);
    // Seed seed(config.communicationMode, common::topology, logger);
    phyLayer->takeOwnership(seed.transferOwnership()); //seed memory is released safely

    //the clock could be renamed as the scheduler TODO
    clock = std::make_unique<Clock>(logger);
    phyLayer->registerAllNodeEvents(*clock);
    clock->start();
    running = true;
    logger.logSystem("Simulation started.");
}

void CommandManager::stopSimulation()
{
    if (!running)
        return;

    logger.logSystem("Stopping simulation...");
    running = false;

    if (clock)
    {
        clock->stop();
        clock.reset();
    }

    phyLayer.reset(); // Frees all node memory
    logger.logSystem("Simulation stopped.");
}

void CommandManager::stop()
{
    stopSimulation();

}

bool CommandManager::isRunning() const
{
    return running;
}
