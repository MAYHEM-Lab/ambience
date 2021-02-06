#include "tos/gfx/bitmap.hpp"

#include <doctest.h>
#include <tos/gfx/truetype.hpp>
#include <tos/gfx/fonts/opensans_regular.hpp>

namespace tos::gfx {
namespace {
TEST_CASE("Truetype font loading works") {
    auto font = font::make(tos::gfx::fonts::opensans_regular);
    REQUIRE(font.valid());
    REQUIRE_EQ(24, font.text_dimensions("hello", 16).width);
    REQUIRE_EQ(13, font.text_dimensions("hello", 16).height);
}

TEST_CASE("Truetype text rendering works") {
    auto font = font::make(tos::gfx::fonts::opensans_regular);

    mono8 bitmap[128 * 24] = {};
    basic_bitmap_view<mono8> view(bitmap, 128, {128, 24});

    auto res = font.render_text("hello", 16, view);
}
} // namespace
} // namespace tos::gfx