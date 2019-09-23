//
// Created by fatih on 9/21/19.
//

#pragma once

#include "dimensions.hpp"

namespace tos::gfx {
template<class CanvasT>
void draw_vertical_line(point from, point to, bool color, CanvasT& canvas) {
    for (auto y = from.y; y < to.y; ++y) {
        canvas.set_pixel(from.x, y, color);
    }
}

template<class CanvasT>
void draw_horizontal_line(point from, point to, bool color, CanvasT& canvas) {
    for (auto x = from.x; x < to.x; ++x) {
        canvas.set_pixel(x, from.y, color);
    }
}
} // namespace tos::gfx