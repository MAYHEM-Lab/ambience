#include <doctest.h>
#include <tos/base64.hpp>

namespace tos {
namespace {
TEST_CASE("Base64 Encoding Works") {
    constexpr uint8_t sample[] = {'h', 'e', 'l', 'l', 'o'};
    auto encoded = base64encode(sample);
    REQUIRE_EQ(8, encoded.size());
    REQUIRE_EQ(tos::raw_cast<const uint8_t>(tos::span("aGVsbG8=").pop_back()),
               tos::span(encoded));
}

TEST_CASE("Base64 Decoding Works") {
    constexpr uint8_t sample[] = "aGVsbG8gd29ybGQgZnJvbSB0b3M=";
    auto decoded = base64decode(tos::span(sample).pop_back());
    REQUIRE_EQ(tos::raw_cast<const uint8_t>(tos::span("hello world from tos").pop_back()),
               tos::span(decoded));
}
} // namespace
} // namespace tos