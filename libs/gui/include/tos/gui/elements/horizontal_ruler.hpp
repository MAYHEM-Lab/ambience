#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
struct horizontal_ruler : public element {
    void draw(const draw_context& ctx);

    view_limits limits(const draw_context&) const {
        return { {0, 1}, {full_extent, 1} };
    }
};
struct vertical_ruler : public element {
    void draw(const draw_context& ctx);

    view_limits limits() const {
        return { {1, 0}, {1, full_extent} };
    }
};

} // namespace tos::gui::elements