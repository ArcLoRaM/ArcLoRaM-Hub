#include "CommandManager.hpp"
#include "Setup/Common.hpp"
#include "Setup/Seed/Seed.hpp"
#include "Node/Node.hpp"
#include "../Packets/Packets.hpp"

CommandManager::CommandManager(Logger &logger_)
    : logger(logger_),
      tcpClient("127.0.0.1", 5000, logger_),
      dispatcher(logger),
      running(false)
{

    logger.setTcpClient(&tcpClient);
    tcpClient.setPacketHandler([this](sf::Packet &p)
                               { dispatcher.onCommand(p); });

    dispatcher.setStopCallback([this]()
                               {
        if(!isRunning()) return;

        logger.logSystem("Stop callback triggered.");
        this->stopSimulation();
        this->dispatcher.clearConfig();

        std::thread([this]() {
            this->waitForLaunchConfig();
        }).detach(); 
    });

    dispatcher.setPingCallback([this]()
                               {
                                //    logger.logSystem("Ping callback triggered, sending pong");
                                    sf::Packet pongBasePacket;
                                    pongPacket pongPck;
                                    pongBasePacket << pongPck;
                                    logger.sendTcpPacket(pongBasePacket); });

    dispatcher.setRestartCallback([this]()
                                  {
        //Todo: when we will add a terminal condition to the simulation (such as Tmax= ...), the simulation wont be "running" but we should be able to restart it
        if(!isRunning()) return;

        logger.logSystem("Restart callback triggered.");
        // Step 1: Stop current simulation
        this->stopSimulation();

        // Step 2: Get the latest launch configuration (already stored)
        auto configOpt = dispatcher.getPendingLaunchConfig();
        if (!configOpt) {
            logger.logSystem("No previous configuration found. Restart aborted.");
            return;
        }

        // Step 3: Launch again with same config
        this->launchSimulation(*configOpt);
    });

    dispatcher.setPauseCallback([this]()
                                {
        if(!isRunning()) return;
        logger.logSystem("Pause callback triggered.");
        this->clock->pause(); });

    dispatcher.setResumeCallback([this]()
                                 {
                if(!isRunning()) return;

        logger.logSystem("Resume callback triggered.");
        this->clock->resume(); });

    tcpClient.setConnectionChangedCallback([this](bool up)
                                           {
    if (!up) {
        logger.logSystem("Connection lost: stopping simulation.");
        this->stopSimulation();
    } else {
        this->dispatcher.clearConfig(); // Just in case
        std::thread([this]() {
            this->waitForLaunchConfig();
        }).detach();

    } });

    initialized = true;
}

CommandManager::~CommandManager()
{

    stop();
    tcpClient.stop();

}

void CommandManager::start()
{

    tcpClient.start();

    //This is done automatically once connection with server is established
    // logger.logSystem("Waiting for launch config from GUI...");
    // waitForLaunchConfig();
}

void CommandManager::waitForLaunchConfig()
{
    if (waitingForConfig.test_and_set())
    {
        logger.logSystem("Already waiting for configuration. Skipping duplicate wait.");
        return;
    }

    std::thread([this]()
                {
    std::optional<LaunchConfig> configOpt;
    logger.logSystem("Waiting for Config....");
    while (!(configOpt = dispatcher.getPendingLaunchConfig()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    launchSimulation(*configOpt);

            waitingForConfig.clear(); // allow future calls
         })
        .detach();
}

void CommandManager::launchSimulation(const LaunchConfig &config)
{
    logger.logSystem("Launching simulation...");

    // Build simulation, TODO: add path loss model here?
    phyLayer = std::make_unique<PhyLayer>(config.distanceThreshold, logger);
    logger.logSystem("Physical Engine Created");

    // Create seed with scenario
    Seed seed(config.topologyLines, config.scenarioType, logger);

    phyLayer->takeOwnership(seed.transferOwnership()); // seed memory is released safely

    logger.logSystem("Topology Deployed");
    clock = std::make_unique<Clock>(logger, std::chrono::milliseconds(common::tickIntervalForClock_ms));
    logger.logSystem("Scheduler Created");

    //kickstarts blueprint-based scheduling:
    phyLayer->registerAllNodeFirstEvent(*clock);

    clock->start();
    running = true;
    logger.logSystem("Simulation started.");
}

void CommandManager::stopSimulation()
{

    running = false;
    if (clock)
    {
        clock->stop();
    }
    phyLayer.reset(); // Frees all node memory
    logger.resetTick();
}

void CommandManager::stop()
{
    stopSimulation();
}

bool CommandManager::isRunning() const
{
    return running;
}
