#pragma once

#include <tos/gfx2.hpp>
#include <limits>

namespace tos::gui {
struct view_limits {
    gfx2::size min;
    gfx2::size max;

    [[nodiscard]]
    bool fixed_width() const {
        return min.width() == max.width();
    }

    [[nodiscard]]
    bool fixed_height() const {
        return min.height() == max.height();
    }
};

constexpr int16_t full_extent = std::numeric_limits<int16_t>::max();
}