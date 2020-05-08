#include <array>
#include <doctest.h>
#include <tos/string.hpp>

namespace tos {
namespace {
TEST_CASE("Split works for ip") {
    std::array<std::string_view, 4> parts;
    split("192.168.0.1", ".", parts.begin());
    REQUIRE_EQ("192", parts[0]);
    REQUIRE_EQ("168", parts[1]);
    REQUIRE_EQ("0", parts[2]);
    REQUIRE_EQ("1", parts[3]);
}

TEST_CASE("Split works for C++ scoping") {
    std::array<std::string_view, 3> parts;
    split("std::vector<int>::value_type", "::", parts.begin());
    REQUIRE_EQ("std", parts[0]);
    REQUIRE_EQ("vector<int>", parts[1]);
    REQUIRE_EQ("value_type", parts[2]);
}
} // namespace
} // namespace tos