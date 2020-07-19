#include <tos/gui/elements/rbox.hpp>

namespace tos::gui::elements {
void rbox::draw(const draw_context& ctx) {
    tos::services::style fill_style(ctx.theme->bg_color);
    ctx.painter->set_style(fill_style);
    ctx.painter->draw_rect(ctx.bounds, radius, true);
}
}