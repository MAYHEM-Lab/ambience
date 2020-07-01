#pragma once

#include <tos/gui/element.hpp>

namespace tos::gui::elements {
struct box : public element {
    void draw(tos::services::painter& painter, const tos::gfx2::rectangle& at);
};
} // namespace tos::gui::elements