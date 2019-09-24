//
// Created by fatih on 9/20/19.
//

#pragma once

#include <cstdint>

namespace tos::gfx {
/**
 * This type represents 2D dimensions.
 */
struct dimensions {
    dimensions() = default;
    constexpr dimensions(int w, int h) : width(w), height(h) {}

    uint16_t width;
    uint16_t height;
};

/**
 * Represents a point in 2D space.
 */
struct point {
    uint16_t x;
    uint16_t y;
};
} // namespace tos::gfx