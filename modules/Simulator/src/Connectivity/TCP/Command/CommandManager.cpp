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
        this->stopSimulation();
        this->dispatcher.clearConfig();

        std::thread([this]() {
            this->waitForLaunchConfig();
        }).detach(); 
    });

    dispatcher.setPingCallback([this]()
                               {
                                    sf::Packet pongBasePacket;
                                    pongPacket pongPck;
                                    pongBasePacket << pongPck;
                                    logger.sendTcpPacket(pongBasePacket); });

    dispatcher.setRestartCallback([this]()
                                  {
        //Todo: when we will add a terminal condition to the simulation (such as Tmax= ...), the simulation wont be "running" but we should be able to restart it
        if(!isRunning()) return;

        // Step 1: Stop current simulation
        this->stopSimulation();

        // Step 2: Get the latest launch configuration (already stored)
        auto configOpt = dispatcher.getPendingLaunchConfig();
        if (!configOpt) {
            logger.logCritical("No previous configuration found. Restart aborted.");
            return;
        }

        // Step 3: Launch again with same config
        this->launchSimulation(*configOpt);
    });

    dispatcher.setPauseCallback([this]()
                                {
        if(!isRunning()) return;
        this->clock->pause(); });

    dispatcher.setResumeCallback([this]()
                                 {
                if(!isRunning()) return;
        this->clock->resume(); });

    tcpClient.setConnectionChangedCallback([this](bool up)
                                           {
    if (!up) {
        logger.logCritical("Connection lost: stopping simulation.");
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

}

void CommandManager::waitForLaunchConfig()
{
    if (waitingForConfig.test_and_set())
    {
        logger.logWarning("Already waiting for configuration. Skipping duplicate wait.");
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

    // === STEP 1: Create MetricsAggregator FIRST (must outlive nodes and clock) ===
    metricsAggregator = std::make_unique<MetricsAggregator>(logger);
    logger.logSystem("Metrics Aggregator Created");

    // === STEP 2: Build Physical Layer ===
    phyLayer = std::make_unique<PhyLayer>(config.distanceThreshold, logger);
    logger.logSystem("Physical Engine Created");

    // === STEP 3: Create and deploy topology ===
    Seed seed(config.topologyLines, config.scenarioType, logger);
    phyLayer->takeOwnership(seed.transferOwnership()); // seed memory is released safely
    logger.logSystem("Topology Deployed");

    // === STEP 4: Set MetricsAggregator on all nodes ===
    phyLayer->setMetricsAggregatorForAllNodes(metricsAggregator.get());
    logger.logSystem("Metrics configured for all nodes");

    // === STEP 5: Create Clock and set MetricsAggregator ===
    clock = std::make_unique<Clock>(logger, std::chrono::milliseconds(common::tickIntervalForClock_ms));
    clock->setMetricsAggregator(metricsAggregator.get());
    logger.logSystem("Scheduler Created");

    // === STEP 6: Start simulation ===
    phyLayer->registerAllNodeFirstEvent(*clock);
    clock->start();
    running = true;
    logger.logSystem("Simulation started.");
}

void CommandManager::stopSimulation()
{
    running = false;

    // === CRITICAL: Destruction order must be REVERSE of creation ===

    // 1. Stop clock (no more metric sampling)
    if (clock)
    {
        clock->stop();
    }

    // 2. Log metrics summary BEFORE destroying nodes (while NodeMetrics are still valid)
    if (metricsAggregator) {
        metricsAggregator->logMetricsSummary();
    }

    // 3. Destroy PhyLayer FIRST (destroys all nodes, which hold metricsAggregator raw pointers)
    phyLayer.reset();
    logger.logSystem("Physical Engine destroyed (all nodes freed)");

    // 4. Destroy Clock SECOND (holds metricsAggregator raw pointer)
    clock.reset();
    logger.logSystem("Scheduler destroyed");

    // 5. Destroy MetricsAggregator LAST (safe to destroy now that no one references it)
    metricsAggregator.reset();
    logger.logSystem("Metrics Aggregator destroyed");

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
