#include <doctest.h>
#include <tos/uuid.hpp>

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
} // namespace
} // namespace tos