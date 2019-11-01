#pragma once 

#include <cstdint>
#include <tos/span.hpp>

namespace tos {
    constexpr static auto crc32_poly = 0x82f63b78;
    
    inline uint32_t crc32(tos::span<const uint8_t> buffer, uint32_t crc = 0, uint32_t poly=crc32_poly)
    {
        crc = ~crc;
        for (auto chr : buffer) {
            crc ^= chr;
            for (int k = 0; k < 8; k++)
                crc = crc & 1 ? (crc >> 1) ^ poly : crc >> 1;
        }
        return ~crc;
    }
}