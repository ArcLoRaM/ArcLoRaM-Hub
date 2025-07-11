#ifndef C2_RCC_UPLINK_SLOT_MANAGER_HPP
#define C2_RCC_UPLINK_SLOT_MANAGER_HPP

#include <vector>

class C2RccUplinkSlotManager {
private:
    std::vector<int> slots;

public:
    C2RccUplinkSlotManager() = default;

    void initializeRandomSlots(int numberOfSlots, int totalSlotsPossible);

    void decrementAllSlots();

    bool canTransmitNow() const;

    void consumeSlot();

    bool hasSlots() const noexcept;

    void reset() noexcept;

    const std::vector<int>& getSlots() const noexcept;
};

#endif // C2_RCC_UPLINK_SLOT_MANAGER_HPP
