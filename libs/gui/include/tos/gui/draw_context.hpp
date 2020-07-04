#pragma once

#include <tos/gui/theme.hpp>
#include <tos/gfx2.hpp>

namespace tos::gui {
struct draw_context {
    gfx2::rectangle update_mask;
    const theme* theme;
};
}