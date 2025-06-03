#include "Clock.hpp"
#include <barrier>
void Clock::start(){
        
        /*
        begins a background thread that repeatedly calls tick() at fixed intervals (tickInterval). 
        The loop sleeps between iterations to ensure consistent timing.
        */

        running = true;
        lastProcessedTime = currentTimeInMilliseconds();
        logger.logMessage(Log("Clock started at: " + std::to_string(lastProcessedTime), true));

        
        clockThread= std::thread([this]() {
            auto nextTick = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());

            // Log printNextTick("nextTIckeValue:"+std::to_string (nextTick.time_since_epoch().count()),true);
            // logger.logMessage(printNextTick);

            while (running) {
                
                
                nextTick += tickInterval;
            //   Log printNextTick2("nextTIckeValue:"+std::to_string( nextTick.time_since_epoch().count()),true);
            //   logger.logMessage(printNextTick2);
                auto tickStart = std::chrono::steady_clock::now();
                tick();
                auto tickEnd = std::chrono::steady_clock::now();

                auto tickDuration = std::chrono::duration_cast<std::chrono::milliseconds>(tickEnd - tickStart);
                if (tickDuration > tickInterval)
                {
                    logger.logMessage(Log("⚠️ Tick " + std::to_string(compteurTick) +
                                              " took " + std::to_string(tickDuration.count()) +
                                              " ms — exceeded interval of " + std::to_string(tickInterval.count()) + " ms, consider increasing the tick interval",
                                          true));
                }
                else
                {
                    std::this_thread::sleep_until(nextTick);
                }
            }
        });
    }

void Clock::stop(){
        running = false;
        if (clockThread.joinable()) {
        clockThread.join();
    }
    }

void Clock::tick() {
    compteurTick++;


    sf::Packet tickPacketReceiver;
    tickPacket tickPacket(compteurTick);
    tickPacketReceiver<<tickPacket;
    logger.sendTcpPacket(tickPacketReceiver);
    // Log callbacks("During tick: "+std::to_string(compteurTick)/*+ ", callbacks found for time: "+std::to_string(time)*/, true);
    // logger.logMessage(callbacks); 
    // //Collect the callbacks during the last processed time and the current time
    //execute them in the order they were scheduled


    int64_t currentTime = currentTimeInMilliseconds();

    // Debugging: Print current and last processed times

    //Log tickLog("Current Time: "+std::to_string(currentTime)+", Last Processed Time: "+std::to_string(lastProcessedTime), true);
   // logger.logMessage(tickLog);
   
    // Collect callbacks to execute between lastProcessedTime and currentTime
    std::vector<CallbackType> callbacksToExecute;

    for (int64_t time = lastProcessedTime; time <= currentTime; ++time) {
        auto range = scheduledCallbacks.equal_range(time);

            for (auto it = range.first; it != range.second; ++it) {
                callbacksToExecute.push_back(it->second); // Collect the callback
            }    
        
    }

    // Update last processed time
    lastProcessedTime = currentTime;
    uint16_t taskCount = static_cast<uint16_t>(callbacksToExecute.size());
    if (taskCount == 0) return;

    logger.logMessage(Log("Tick " + std::to_string(compteurTick) +
                          ": dispatching " + std::to_string(taskCount) + " transitions", true));


    std::barrier syncPoint(taskCount, [this]() {
        logger.logMessage(Log("All threads are ready to execute their transitions", true));
    });

    //needs to be provisionned dynamically
    auto doneMutex = std::make_shared<std::mutex>();
    auto doneCv = std::make_shared<std::condition_variable>();
    auto doneCount = std::make_shared<std::atomic<size_t>>(0);
    auto startTime = std::chrono::steady_clock::now();
    // Put all the callbacks i the thread pool so they can be executed concurrently
    for (auto& callback : callbacksToExecute) {
        threadPool.enqueue([&syncPoint, doneCv, doneMutex, doneCount, cb = std::move(callback)]() mutable {
            syncPoint.arrive_and_wait(); //wait at the barrier until all threads are ready
            cb(); // The transition logic
            doneCount->fetch_add(1); 
            std::lock_guard<std::mutex> lock(*doneMutex);
            doneCv->notify_one();
        });
    }
    {
        std::unique_lock<std::mutex> lock(*doneMutex);
        doneCv->wait(lock, [&]() {
            return doneCount->load() == taskCount;
        });
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    if (duration > std::chrono::milliseconds(100)) {
        logger.logMessage(Log("⚠️ Tick " + std::to_string(compteurTick) +
                              " took " + std::to_string(duration.count()) + " ms", true));
    }

    assert(doneCount->load() == taskCount && "Error: Not all scheduled state transitions completed!");



}

void Clock::scheduleCallback(int64_t activationTime, CallbackType callback){
        //put the callback in the list of events at the given time
        //for one time stamp, there can multiple events (one for each node)
        //emplace and move are used to avoid copying the callback
        
        scheduledCallbacks.emplace(activationTime, std::move(callback));
    }

int64_t Clock::currentTimeInMilliseconds(){
        //returns the time that passed since the "epoch" (1st January 1970) in ms
        auto now = std::chrono::system_clock::now().time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }






ThreadPool::ThreadPool(int numThreads) : stop(false) {
    for (int i = 0; i < numThreads; ++i) {
        workers.emplace_back([this]() { worker(); });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        if (worker.joinable())
            worker.join();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        tasks.emplace(std::move(task));
    }
    condition.notify_one();
}

void ThreadPool::worker() {
    while (!stop) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}
