#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <functional>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include "../../Connectivity/Logger/Logger.hpp"
#include <string>
#include "../../Connectivity/TCP/packets.hpp"



class ThreadPool {
public:
    explicit ThreadPool(int numThreads);
    ~ThreadPool();

    void enqueue(std::function<void()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    void worker();
};


class Clock {


private:
    std::atomic<bool> running;
    std::chrono::milliseconds tickInterval; // Configurable tick interval: decrease for more frequent ticks -> closer to a continuous simulation
                                            //default it 10ms
    using CallbackType = std::function<void()>;
    std::multimap<int64_t, CallbackType> scheduledCallbacks; //stores the calls of onTimeChange() for each node at the activation times
                                                             //onTimeChange() will call the appropriate stateTransitionFunction
                                                             

    // Current time in milliseconds
    int64_t currentTimeInMilliseconds() ;
    std::mutex callbackMutex;
    std::thread clockThread;
    Logger& logger;
    
    int64_t lastProcessedTime;
    void tick();
    unsigned int compteurTick;

    ThreadPool threadPool;


public:
    Clock( Logger& logger,int baseUnitMilliseconds,int poolSize) 
        : running(false), tickInterval(baseUnitMilliseconds), lastProcessedTime(0),logger(logger), threadPool(poolSize) {
            compteurTick=0;
        }

    void start() ;

    void stop();

    void scheduleCallback(int64_t activationTime, CallbackType callback) ;
};

