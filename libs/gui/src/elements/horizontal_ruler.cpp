#include <tos/gui/elements/horizontal_ruler.hpp>

namespace tos::gui::elements {
void horizontal_ruler::draw(services::painter& painter, const gfx2::rectangle& at) {
    tos::services::style fill_style(tos::gfx2::binary_color{false});
    painter.set_style(fill_style);
    painter.draw_line(
        {at.corner(), {at.corner().x() + at.dims().width(), at.corner().y()}});
}
}