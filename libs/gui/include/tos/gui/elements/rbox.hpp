#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
struct rbox : public element {
    int radius;

    void draw(const draw_context& ctx);

    view_limits limits(const draw_context&) const {
        return { {0, 0}, {full_extent, full_extent} };
    }
};
} // namespace tos::gui::elements