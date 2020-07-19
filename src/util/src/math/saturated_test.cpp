#include <algorithm>
#include <cstdint>
#include <doctest.h>
#include <tos/math/saturated.hpp>

namespace tos::math {
namespace {
TEST_CASE("Saturated add for i16 works") {
    for (int32_t i = std::numeric_limits<int16_t>::min();
         i < std::numeric_limits<int16_t>::max();
         ++i) {
        for (int32_t j = std::numeric_limits<int16_t>::min();
             j < std::numeric_limits<int16_t>::max();
             j += 100) {
            REQUIRE_EQ(std::clamp<int32_t>(i + j,
                                           std::numeric_limits<int16_t>::min(),
                                           std::numeric_limits<int16_t>::max()),
                       add_saturating(static_cast<int16_t>(i), static_cast<int16_t>(j)));
        }
    }
}
} // namespace
} // namespace tos::math