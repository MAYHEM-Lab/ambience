#pragma once

#include <cstdint>
#include <limits>
#include <algorithm>

namespace tos::math {
int16_t add_saturating(int16_t a, int16_t b) {
    return std::clamp<int32_t>(
        a + b, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());
}
} // namespace tos::math