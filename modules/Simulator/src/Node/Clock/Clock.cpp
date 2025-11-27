#include "Clock.hpp"
#include "../../Metrics/MetricsAggregator.hpp"
#include "../../Metrics/MetricsExporter.hpp"
#include "../../Metrics/MetricsConfig.hpp"
#include <barrier>



 Clock::Clock(Logger& logger, ms tickPeriod)
  : logger(logger), tickPeriod(tickPeriod) {}


    Clock::~Clock()
    {
        stop();
    }

void Clock::start(){
        
        /*
        begins a background thread that repeatedly calls tick() at fixed intervals (tickInterval). 
        The loop sleeps between iterations to ensure consistent timing.
        */



 std::scoped_lock lk(clockMutex);
    if (running.load()) return;

    running.store(true);
    paused.store(false);

    tNowMs   = 0;
    tLastMs  = -1;
    tickCount = 0;

    // Initialize metrics exporter
    if (!metricsExporter) {
        metricsExporter = std::make_unique<MetricsExporter>();
    }

 // spawn the clock thread; run() implements the virtual-time loop
    clockThread = std::jthread([this](std::stop_token st){ run(st); });
    }

void Clock::run(std::stop_token st) {
    while (!st.stop_requested()) {
                uint64_t start = 0, end = 0;

        // 1) Pause gate + advance logical time under lock
        {
            std::unique_lock lk(clockMutex);
            cv.wait(lk, st, [&]{ return !paused.load() || st.stop_requested(); });
            if (st.stop_requested() || !running.load()) break;

            // advance logical time by exactly one tick
            start = tNowMs;
            tNowMs += tickPeriod.count();
            end = tNowMs;
            ++tickCount;
            // release lock before heavy work
        }
            sf::Packet tickPacketReceiver;
            tickPacket tickPacket(tickCount);
            tickPacketReceiver<<tickPacket;
            logger.sendTcpPacket(tickPacketReceiver);
        // 2) Execute (start, end] under lock to touch maps safely
        {
       // -----------------------------
     // Phase 1: Execute all transitions (nodes wake up, stay asleep...)
    // -----------------------------

            executeCallbacksInRange(stateTransitions,            start, end);
    // -----------------------------
    // Phase 2: Execute all communication steps (nodes handle communication)
    // -----------------------------
 
            executeCommunicationInRange(communicationSteps,      start, end);
    // -----------------------------
    // Phase 3: Packets transmission (deals with physical layer)
    // -----------------------------

            executeCallbacksInRange(transmissionStartCallbacks,  start, end);
            executeCallbacksInRange(transmissionEndCallbacks,    start, end);

        // === METRICS: Periodic sampling ===
        if (metricsAggregator &&
            tickCount % MetricsConfig::DEFAULT_SAMPLING_INTERVAL_TICKS == 0) {
            metricsAggregator->sampleAllNodes(end);
        }

        // === METRICS: Export to GUI ===
        if (metricsAggregator && metricsExporter &&
            tickCount % MetricsConfig::METRICS_EXPORT_INTERVAL_TICKS == 0) {
            metricsExporter->exportAllMetrics(metricsAggregator, tickCount, logger);
        }

        {
            std::scoped_lock lk(clockMutex);
            tLastMs = end;
            logger.setCurrentTick(end);
        }
        }

        // 3) Optional throttle; interruptible by pause/stop
        // use an interruptible wait so stop()/pause() wakes promptly
        std::unique_lock lk(clockMutex);
        cv.wait_for(lk, st, tickPeriod, [&]{ 
            return paused.load() || st.stop_requested();
         });
    }
}


void Clock::stop(){
  {
        std::scoped_lock lk(clockMutex);
        if (!running.load()) return;
        running.store(false);
    }
    if (clockThread.joinable()) {
        clockThread.request_stop();
        cv.notify_all();      // wake if paused or waiting
        clockThread.join();
    }
    }

// ---- Control ----
void Clock::pause() {
    {
        std::scoped_lock lk(clockMutex);
        logger.logSystem("Clock paused.");

        if (!running.load()) return;
        paused.store(true);
    }
    cv.notify_all(); // wake immediately
}

void Clock::resume() {
    {
        std::scoped_lock lk(clockMutex);
        logger.logSystem("Clock resumed.");

        if (!running.load()) return;
        paused.store(false);
    }
    cv.notify_all();          // release waiters
}

void Clock::scheduleStateTransition(int64_t activationTime, CallbackType callback){
        //put the callback in the list of events at the given time
        //for one time stamp, there can multiple events (one for each node)
        //emplace and move are used to avoid copying the callback
        std::scoped_lock lk(clockMutex);
        stateTransitions.emplace(activationTime, std::move(callback));
    }


void Clock::scheduleCommunicationStep(int64_t time, std::shared_ptr<Node> node) {
    std::scoped_lock lk(clockMutex);
    communicationSteps.emplace(time, std::move(node));
}



void Clock::executeCommunicationInRange(
    std::multimap<int64_t, std::shared_ptr<Node>>& map,
    int64_t start, int64_t end)
{
    
    auto it = map.begin();
    while (it != map.end() && it->first <= end) {
        if (it->first <= start) {
        std::cerr << "❗ Event scheduled at or before already-processed time: " << it->first << " <= " << start << "\n";
    }


        if (it->first > start) {
            it->second->handleCommunication();
            logger.logEvent(it->second->getId(), "HandleComm");
            it = map.erase(it);
        } else {
            ++it;
        }
    }

    //Alternative proceeding, but does nto fire warning log in case of past events
    //todo implement this?
    // auto it    = map.upper_bound(start);
    // auto itEnd = map.upper_bound(end);
    // for (; it != itEnd; it = map.erase(it)) {
    //     if (it->second) {
    //         it->second->handleCommunication();
    //     }
    // }
}

void Clock::executeCallbacksInRange(std::multimap<int64_t, CallbackType>& map, int64_t start, int64_t end) {
    auto it = map.begin();
    // exclusive of start, inclusive of end
    while (it != map.end() && it->first <= end) {
        if (it->first <= start) {
          std::cerr << "❗ Event scheduled at or before already-processed time: " << it->first << " <= " << start << "\n";
            it = map.erase(it);
        }
        if (it->first > start) {
            it->second(); //execute the callback
            it = map.erase(it);
        } else {
            ++it;
        }
    }


    //alternative:

    //     // Execute events in (start, end], erasing as we go
    // auto it    = map.upper_bound(start);
    // auto itEnd = map.upper_bound(end);
    // for (; it != itEnd; it = map.erase(it)) {
    //     it->second(); // invoke callback
    // }
}

int64_t Clock::currentTimeInMilliseconds(){
        
    std::scoped_lock lk(clockMutex);
    return tNowMs;
}



void Clock::scheduleTransmissionStart(int64_t time, CallbackType callback) {
    std::scoped_lock lk(clockMutex);
    transmissionStartCallbacks.emplace(time, std::move(callback));
}

void Clock::scheduleTransmissionEnd(int64_t time, CallbackType callback) {
    std::scoped_lock lk(clockMutex);    
    transmissionEndCallbacks.emplace(time, std::move(callback));
}



std::multimap<int64_t, std::shared_ptr<Node>> Clock::getCommunicationStepsSnapshot() const {
        std::scoped_lock lk(clockMutex);
        return communicationSteps; // returns a copy
}