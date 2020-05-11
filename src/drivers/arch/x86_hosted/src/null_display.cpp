#include <arch/display.hpp>

namespace tos::x86 {
struct display_impl {};
display::display(const gfx::dimensions &) {}
display::~display() = default;

namespace {
class null_painter : public gfx::painter {
public:
    void draw(const gfx::line&, const gfx::fixed_color&) override {
    }
    void draw(const gfx::rectangle&, const gfx::fixed_color&) override {
    }
};
}

std::unique_ptr<gfx::painter> display::get_painter() {
    return std::make_unique<null_painter>();
}
}