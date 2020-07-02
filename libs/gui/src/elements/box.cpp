#include <tos/gui/elements/box.hpp>

namespace tos::gui::elements {
void box::draw(services::painter& painter, const gfx2::rectangle& at) {
    tos::services::style fill_style(tos::gfx2::rgb8{255, 255, 255});
    painter.set_style(fill_style);
    painter.draw_rect(at, 0, true);
}
} // namespace tos::gui::elements