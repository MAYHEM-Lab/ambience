#include <array>
#include <doctest.h>
#include <tos/mem_stream.hpp>


namespace tos {
namespace {
TEST_CASE("Output memory streams work") {
    std::array<uint8_t, 128> buf;
    omemory_stream str(buf);
    const uint8_t data[] = {'h', 'e', 'l', 'l', 'o'};
    str.write(raw_cast<const uint8_t>(span(data)));
    REQUIRE_EQ(5, str.get().size());
    REQUIRE_EQ(span<const uint8_t>(data), str.get());
}
} // namespace
} // namespace tos