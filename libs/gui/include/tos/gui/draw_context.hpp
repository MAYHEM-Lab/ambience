#pragma once

#include <tos/gfx2/painter.hpp>
#include <tos/gui/theme.hpp>
#include <tos/gfx2.hpp>

namespace tos::gui {
struct draw_context {
    services::painter* const painter;

    gfx2::rectangle update_mask;

    const basic_theme* theme;
    gfx2::rectangle bounds;
};
}