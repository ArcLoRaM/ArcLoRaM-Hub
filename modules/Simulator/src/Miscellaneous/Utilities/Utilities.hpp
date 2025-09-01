#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <random>
#include <algorithm>
#include "../../Setup/Common.hpp"

#include <sstream>
#include <iomanip>
#include "../../Setup/Common.hpp"
#include <unordered_map>
#include <chrono>
#include <algorithm>

namespace packet_tool
{

    //======================================
    // Byte & Field Operations
    //======================================

    // Convert a uint32_t into a little-endian byte vector of given size
    inline std::vector<uint8_t> decimalToBytes(uint32_t decimalValue, size_t byteCount)
    {
        std::vector<uint8_t> bytes(byteCount, 0);
        for (size_t i = 0; i < byteCount; ++i)
        {
            bytes[i] = static_cast<uint8_t>(decimalValue & 0xFF);
            decimalValue >>= 8;
        }
        return bytes;
    }

    // Extract the integer value of a named field using a FieldMap
    inline uint32_t extractBytesFromField(
        const std::vector<uint8_t> &packet,
        std::string_view fieldName,
        const common::FieldMap &fieldMap)
    {
        auto it = fieldMap.find(fieldName);
        if (it == fieldMap.end())
        {
            throw std::invalid_argument("Field name not recognized: " + std::string(fieldName));
        }

        const auto &[offset, size] = it->second;

        if (offset + size > packet.size())
        {
            throw std::out_of_range("Packet too small for field: " + std::string(fieldName));
        }

        uint32_t value = 0;
        for (size_t i = 0; i < size; ++i)
        {
            value |= static_cast<uint32_t>(packet[offset + i]) << (8 * i); // Little-endian
        }

        return value;
    }

    // Return a string representation of field names and their values from the packet
    inline std::string detailedBytesToString(
        const std::vector<uint8_t> &packet,
        const common::FieldMap &fieldMap)
    {
        std::ostringstream oss;
        std::vector<std::string_view> fieldNames;
        fieldNames.reserve(fieldMap.size());

        for (const auto &[fieldName, _] : fieldMap)
        {
            fieldNames.push_back(fieldName);
        }

        std::reverse(fieldNames.begin(), fieldNames.end());

        for (const auto &fieldName : fieldNames)
        {
            uint32_t value = extractBytesFromField(packet, fieldName, fieldMap);
            oss << fieldName << ": " << value << " - ";
        }

        std::string result = oss.str();
        if (!result.empty())
        {
            result.erase(result.size() - 3); // Remove trailing " - "
        }

        return result;
    }

    // Convert a packet into hexadecimal string representation
    inline std::string to_hex(const std::vector<uint8_t> &packet)
    {
        std::ostringstream oss;
        for (uint8_t byte : packet)
        {
            oss << std::hex << std::setw(2) << std::setfill('0')
                << static_cast<int>(byte) << " ";
        }
        return oss.str();
    }
    // Helper lambda to append vectors
    inline auto appendVector = [](std::vector<uint8_t> &dest, const std::vector<uint8_t> &src)
    {
        dest.insert(dest.end(), src.begin(), src.end());
    };

    inline common::PacketType getPacketType(
        const std::vector<uint8_t> &packet,
        const common::FieldMap &map)
    {
        return static_cast<common::PacketType>(
            packet_tool::extractBytesFromField(packet, common::field_names::type, map));
    }

}

namespace simulation_util
{

    inline int computeRandomNbBeaconPackets(int min, int max)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(min, max);
        return dis(gen);
    }

    inline std::vector<int> selectRandomSlots(int m, int nbSlotsPossible)
    {

        // Step 1: Create a vector of slots [0, 1, ..., n-1]
        std::vector<int> slots(nbSlotsPossible);
        for (int i = 0; i < nbSlotsPossible; ++i)
        {
            slots[i] = i;
        }

        // Step 2: Shuffle the vector randomly
        std::random_device rd;  // Seed for random number generator
        std::mt19937 rng(rd()); // Mersenne Twister RNG
        std::shuffle(slots.begin(), slots.end(), rng);

        // Step 3: Take the first m slots
        std::vector<int> selected(slots.begin(), slots.begin() + m);

        // Step 4: Sort the selected slots in ascending order
        std::sort(selected.begin(), selected.end());

        return selected;
    }

}
