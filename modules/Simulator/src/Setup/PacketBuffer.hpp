#pragma once

#include "Common.hpp"
#include "../Metrics/MetricsTypes.hpp"
#include <vector>
#include <deque>
#include <cstdint>
#include <optional>

namespace packet_buffer {

/**
 * Generic packet metadata and payload container
 * Supports originated and forwarded packets for all node types
 */
struct PacketBuffer {
    // === Packet Identity ===
    uint16_t localPacketId = 0;              // Local ID within link (sender, receiver)
    std::optional<GlobalPacketID> globalPacketId; // Global tracking ID for metrics
                                                   // nullopt for originated (assigned at transmission)
                                                   // has_value for forwarded (from received packet)

    // === Packet Classification ===
    enum class PacketOrigin {
        Originated,     // Created by this node (C1, C2)
        Forwarded       // Received and needs forwarding (C2)
    };
    PacketOrigin origin;

    // === Payload ===
    std::vector<uint8_t> payload;            // Actual data to transmit

    // === Timing (for queue delay metrics) ===
    int64_t arrivalTime = 0;                 // When packet entered buffer (ms)
};

/**
 * Simple FIFO packet queue with search/remove capabilities
 * Maintains proper ordering across originated and forwarded packets
 */
class PacketQueue {
private:
    std::deque<PacketBuffer> queue_;

public:
    PacketQueue() = default;

    /**
     * Add packet to end of queue (FIFO)
     * @param packet Packet to enqueue (will be moved)
     * @return true if successful
     */
    bool push(PacketBuffer&& packet) {
        queue_.push_back(std::move(packet));
        return true;
    }

    /**
     * Remove and return first packet from queue
     * @return Packet if queue not empty, nullopt otherwise
     */
    std::optional<PacketBuffer> pop() {
        if (queue_.empty()) {
            return std::nullopt;
        }
        PacketBuffer packet = std::move(queue_.front());
        queue_.pop_front();
        return packet;
    }

    /**
     * Find packet by local ID (for ACK matching)
     * @param localId Local packet ID to search for
     * @return Pointer to packet if found, nullptr otherwise
     */
    PacketBuffer* findByLocalId(uint16_t localId) {
        for (auto& pkt : queue_) {
            if (pkt.localPacketId == localId) {
                return &pkt;
            }
        }
        return nullptr;
    }

    /**
     * Remove packet by local ID (immediate removal on ACK)
     * @param localId Local packet ID to remove
     * @return true if packet was found and removed
     */
    bool removeByLocalId(uint16_t localId) {
        for (auto it = queue_.begin(); it != queue_.end(); ++it) {
            if (it->localPacketId == localId) {
                queue_.erase(it);
                return true;
            }
        }
        return false;
    }

    /**
     * Check if queue is empty
     */
    bool empty() const {
        return queue_.empty();
    }

    /**
     * Get number of packets in queue
     */
    size_t size() const {
        return queue_.size();
    }

    /**
     * Peek at next packet without removing (for inspection)
     * @return Pointer to first packet if queue not empty, nullptr otherwise
     */
    const PacketBuffer* peek() const {
        if (queue_.empty()) {
            return nullptr;
        }
        return &queue_.front();
    }
};

} // namespace packet_buffer
