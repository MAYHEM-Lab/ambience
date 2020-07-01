#include <tos/gui/elements/rbox.hpp>

namespace tos::gui::elements {
void rbox::draw(tos::services::painter& painter, const tos::gfx2::rectangle& at) {
    tos::services::style fill_style(tos::gfx2::binary_color{false});
    painter.set_style(fill_style);
    painter.draw_rect(at, radius, true);
}
}