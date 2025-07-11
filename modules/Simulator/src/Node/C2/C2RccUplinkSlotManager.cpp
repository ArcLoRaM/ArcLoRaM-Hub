#include "C2RccUplinkSlotManager.hpp"
#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>

void C2RccUplinkSlotManager::initializeRandomSlots(int numberOfSlots, int totalSlotsPossible) {
   
    //choose numberofSlots randomly between totalSlotsPossible 
    slots.resize(totalSlotsPossible);
    std::iota(slots.begin(), slots.end(), 0);

    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(slots.begin(), slots.end(), rng);

    numberOfSlots = std::min(numberOfSlots, totalSlotsPossible);
    slots = std::vector<int>(slots.begin(), slots.begin() + numberOfSlots);
    std::sort(slots.begin(), slots.end());
}

void C2RccUplinkSlotManager::decrementAllSlots() {
    for (int& slot : slots) {
        --slot;
    }
}

bool C2RccUplinkSlotManager::canTransmitNow() const {
    return !slots.empty() && slots.front() == 0;
}

void C2RccUplinkSlotManager::consumeSlot() {
    if (slots.empty() || slots.front() != 0) {
        throw std::logic_error("Invalid attempt to consume a slot not ready");
    }
    slots.erase(slots.begin());
}

bool C2RccUplinkSlotManager::hasSlots() const noexcept {
    return !slots.empty();
}

void C2RccUplinkSlotManager::reset() noexcept {
    slots.clear();
}

const std::vector<int>& C2RccUplinkSlotManager::getSlots() const noexcept {
    return slots;
}
