#include "CommandManager.hpp"
#include "Setup/Common.hpp"
#include "Setup/Seed/Seed.hpp"
#include "Node/Node.hpp"

CommandManager::CommandManager(Logger &logger_)
    : logger(logger_),
      tcpClient("127.0.0.1", 5000,logger_),
      dispatcher(logger),
      listener(6000, [this](sf::Packet &p)
               { dispatcher.onCommand(p); }, logger),
      running(false)
{

    logger.setTcpClient(&tcpClient);
    logger.enableFileOutput("output/log_output.txt");
    logger.enableColorOutput(true);

    dispatcher.setStopCallback([this]()
                               { 
                             logger.logSystem("Stop callback triggered.");

                                // this->stopSimulation();
                             });

    dispatcher.setPingCallback([this]()
                               {
                                   logger.logSystem("Ping callback triggered.");
                                   // Optional: reply to GUI with status
                               });

    dispatcher.setRestartCallback([this]() {
            logger.logSystem("Restart callback triggered.");
            this->stopSimulation();
    });
}

CommandManager::~CommandManager()
{
    stop();
    logger.stop(); // Must be stopped last
}

void CommandManager::start()
{
    listener.start();
    logger.start();

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

    // Notify GUI of system parameters
    sf::Packet sysPacket;
    systemPacket sys(common::distanceThreshold, common::communicationMode);
    sysPacket << sys;
    logger.sendTcpPacket(sysPacket);

    // Build simulation
    phyLayer = std::make_unique<PhyLayer>(common::distanceThreshold, logger);
    Seed seed(common::communicationMode, common::topology, logger);
    phyLayer->takeOwnership(seed.transferOwnership());

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
    listener.stop();
}

bool CommandManager::isRunning() const
{
    return running;
}
