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
        logger.logSystem("Connection re-established.");
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
    logger.logSystem("Launching simulation...");

    // Notify GUI of system parameters, information is sent back as a security check
    sf::Packet sysPacket;
    systemPacket sys(common::distanceThreshold, common::communicationMode);
    sysPacket << sys;
    logger.sendTcpPacket(sysPacket);

    // Build simulation
    phyLayer = std::make_unique<PhyLayer>(common::distanceThreshold, logger);
    Seed seed(common::communicationMode, common::topology, logger);
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
