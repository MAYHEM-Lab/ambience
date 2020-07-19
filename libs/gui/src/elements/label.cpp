#include <tos/gui/elements/label.hpp>

namespace tos::gui::elements {
void label::draw(const draw_context& ctx) {
    tos::services::style fill_style(ctx.theme->fg_color);
    ctx.painter->set_style(fill_style);
    ctx.painter->draw_text(m_str, ctx.bounds.corner());
}
}