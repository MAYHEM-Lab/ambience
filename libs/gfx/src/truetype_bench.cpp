#include <tos/gfx/truetype.hpp>
#include <tos/ubench/bench.hpp>
#include <tos/ubench/state.hpp>
#include <tos/gfx/fonts/opensans_regular.hpp>

namespace tos::gfx {
namespace {
void BM_TrueType_RenderText_Hello(bench::any_state& state) {
    auto font = font::make(tos::gfx::fonts::opensans_regular);

    mono8 bitmap[128 * 24] = {};
    basic_bitmap_view<mono8> view(bitmap, 128, {128, 24});

    for (auto _ : state) {
        auto res = font.render_text("hello", 16, view);
    }
}

BENCHMARK(BM_TrueType_RenderText_Hello);

void BM_TrueType_RenderText_HelloWorld(bench::any_state& state) {
    auto font = font::make(tos::gfx::fonts::opensans_regular);

    mono8 bitmap[128 * 24] = {};
    basic_bitmap_view<mono8> view(bitmap, 128, {128, 24});

    for (auto _ : state) {
        auto res = font.render_text("hello world", 16, view);
    }
}

BENCHMARK(BM_TrueType_RenderText_HelloWorld);
} // namespace
} // namespace tos::gfx