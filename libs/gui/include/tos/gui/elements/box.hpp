#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
struct box : public element {
    view_limits limits(const draw_context&) const {
        return { {0, 0}, {full_extent, full_extent} };
    }

    void draw(const draw_context& ctx);
};
} // namespace tos::gui::elements