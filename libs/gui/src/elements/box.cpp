#include <tos/gui/elements/box.hpp>

namespace tos::gui::elements {
void box::draw(const draw_context& ctx) {
    tos::services::style fill_style(ctx.theme->bg_color);
    ctx.painter->set_style(fill_style);
    ctx.painter->draw_rect(ctx.bounds, 0, true);
}
} // namespace tos::gui::elements