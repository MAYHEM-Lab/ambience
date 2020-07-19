#include <tos/gui/elements/horizontal_ruler.hpp>

namespace tos::gui::elements {
void horizontal_ruler::draw(const draw_context& ctx) {
    tos::services::style fill_style(ctx.theme->line_color);
    ctx.painter->set_style(fill_style);
    ctx.painter->draw_line({ctx.bounds.corner(),
                            {ctx.bounds.corner().x() + ctx.bounds.dims().width(),
                             ctx.bounds.corner().y()}});
}

void vertical_ruler::draw(const draw_context& ctx) {
    tos::services::style fill_style(ctx.theme->line_color);
    ctx.painter->set_style(fill_style);
    ctx.painter->draw_line({ctx.bounds.corner(),
                            {ctx.bounds.corner().x(),
                             ctx.bounds.corner().y() + ctx.bounds.dims().height()}});
}
} // namespace tos::gui::elements