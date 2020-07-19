#pragma once

#include <tos/gui/element.hpp>
#include <tos/gfx2/utility.hpp>
#include <tos/gui/decorators.hpp>

namespace tos::gui::elements {
struct placeholder : public element {
    view_limits limits(const draw_context&) const {
        return { {0, 0}, {full_extent, full_extent} };
    }

    void draw(const draw_context& ctx) {
        tos::services::style fill_style(ctx.theme->bg_color);
        ctx.painter->set_style(fill_style);
        ctx.painter->draw_rect(ctx.bounds, 0, true);

        auto line_style = tos::services::style(ctx.theme->fg_color);
        ctx.painter->set_style(line_style);

        ctx.painter->draw_rect(ctx.bounds, 0, false);
        ctx.painter->draw_line({ctx.bounds.corner(), other_corner(ctx.bounds)});
    }
};
}