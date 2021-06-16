#include <doctest.h>
#include <tos/hex.hpp>

namespace tos {
namespace {
TEST_CASE("byte_from_hex works") {
    {
        uint8_t byte = detail::byte_from_hex("00");
        REQUIRE_EQ(0x00, byte);
    }
    {
        uint8_t byte = detail::byte_from_hex("11");
        REQUIRE_EQ(0x11, byte);
    }
    {
        uint8_t byte = detail::byte_from_hex("25");
        REQUIRE_EQ(0x25, byte);
    }
}

TEST_CASE("hex to bytes works") {
    REQUIRE_EQ(std::array<uint8_t, 3>{0x00, 0x61, 0x73}, hex_to_bytes("006173"));
}
} // namespace
} // namespace tos