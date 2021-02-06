#include <doctest.h>
#include <tos/math/nearest_power_of_two.hpp>

namespace tos::math {
namespace {
TEST_CASE("Nearest power of two works") {
    REQUIRE_EQ(0, nearest_power_of_two(1));
    REQUIRE_EQ(1, nearest_power_of_two(2));
    REQUIRE_EQ(2, nearest_power_of_two(3));
    REQUIRE_EQ(10, nearest_power_of_two(1024));
    REQUIRE_EQ(20, nearest_power_of_two(1024 * 1024));
}
} // namespace
} // namespace tos::math