#pragma once

#include <tos/gfx/dimensions.hpp>
#include <tos/gfx/color.hpp>

namespace tos::gfx {
struct line {
    point from; // Bottom left point
    point to;   // Top right point
};

struct fixed_color {
    rgb8 color;
};

struct painter {
    virtual void draw(const line& line, const fixed_color& col) = 0;

    virtual void draw(const rectangle& rect, const fixed_color& col) = 0;

    virtual ~painter() = default;
};
}