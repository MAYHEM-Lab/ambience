//
// Created by fatih on 9/20/19.
//

#pragma once

#include <cstdint>
#include <algorithm>

namespace tos::gfx {
/**
 * This type represents 2D dimensions.
 */
struct dimensions {
    dimensions() = default;
    constexpr dimensions(int w, int h)
        : width(w)
        , height(h) {
    }

    uint16_t width;
    uint16_t height;

    constexpr size_t area() const {
        return height * width;
    }
};

constexpr bool operator==(const dimensions& l, const dimensions& r) {
    return l.width == r.width && l.height == r.height;
}
/**
 * Represents a point in 2D space.
 */
struct point {
    uint16_t x;
    uint16_t y;
};

constexpr bool operator==(const point& l, const point& r) {
    return l.x == r.x && l.y == r.y;
}

struct rectangle {
    point corner; // bottom left corner
    dimensions size;

    constexpr point other_corner() const {
        return point{uint16_t(corner.x + size.width), uint16_t(corner.y + size.height)};
    }

    constexpr void set_other_corner(const point& other_corner) {
        size = {std::max(0, other_corner.x - corner.x),
                std::max(0, other_corner.y - corner.y)};
    }
};

constexpr bool operator==(const rectangle& l, const rectangle& r) {
    return l.corner == r.corner && l.size == r.size;
}

/**
 * Returns the intersection of the given two rectangles.
 *
 * If the two rectangles don't intersect, the dimensions of the returned
 * rectangle will be zero.
 */
constexpr rectangle intersection(const rectangle& a, const rectangle& b);

/**
 * Returns the union of the given two rectangles.
 */
constexpr rectangle bound(const rectangle& a, const rectangle& b);
} // namespace tos::gfx

// impl

namespace tos::gfx {
constexpr rectangle intersection(const rectangle& a, const rectangle& b) {
    rectangle r{};
    r.corner = point{std::max(a.corner.x, b.corner.x), std::max(a.corner.y, b.corner.y)};
    auto other_corner = point{std::min(a.other_corner().x, b.other_corner().x),
                              std::min(a.other_corner().y, b.other_corner().y)};
    r.set_other_corner(other_corner);
    return r;
}

constexpr rectangle bound(const rectangle& a, const rectangle& b) {
    rectangle r{};
    r.corner = point{std::min(a.corner.x, b.corner.x), std::min(a.corner.y, b.corner.y)};
    auto other_corner = point{std::max(a.other_corner().x, b.other_corner().x),
                              std::max(a.other_corner().y, b.other_corner().y)};
    r.set_other_corner(other_corner);
    return r;
}

static_assert(intersection(rectangle{{5, 5}, {30, 30}}, rectangle{{135, 245}, {110, 4}})
                  .size == dimensions{0, 0});

static_assert(rectangle{{4, 4}, {4, 5}} ==
              intersection(rectangle{{4, 3}, {8, 8}}, rectangle{{3, 4}, {5, 5}}));

static_assert(rectangle{{3, 3}, {5, 5}} ==
              intersection(rectangle{{2, 2}, {8, 8}}, rectangle{{3, 3}, {5, 5}}));

static_assert(rectangle{{3, 3}, {5, 5}} ==
              intersection(rectangle{{3, 3}, {8, 8}}, rectangle{{3, 3}, {5, 5}}));
} // namespace tos::gfx