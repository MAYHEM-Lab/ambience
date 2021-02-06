#pragma once

#include <cstdint>
namespace tos::math {
constexpr int nearest_power_of_two(uint32_t val) {
    int i = 0;
    uint32_t accum = 1;
    for (; accum < val; ++i, accum <<= 1)
        ;
    return i;
}
} // namespace tos::math