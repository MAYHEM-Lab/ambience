#include "tos/gfx/color.hpp"

#include <doctest.h>
#include <tos/gfx/bitmap.hpp>

namespace tos::gfx {
namespace {
TEST_CASE("Bitmap view works") {
    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    basic_bitmap_view<uint8_t> view(data, 4, {4, 4});
    REQUIRE_EQ(0, view.at({0, 0}));
    REQUIRE_EQ(1, view.at({1, 0}));

    auto window = view.slice({{1, 1}, {2, 2}});
    REQUIRE_EQ(2, window.dims().width);
    REQUIRE_EQ(4, window.stride());
    REQUIRE_EQ(5, window.at({0, 0}));
    REQUIRE_EQ(10, window.at({1, 1}));
}
} // namespace
} // namespace tos::gfx