#pragma once

#include <cmath>
#include <tos/gfx2.hpp>

namespace tos::gfx2 {
inline point other_corner(const rectangle& rect) {
    return {rect.corner().x() + rect.dims().width(),
            rect.corner().y() + rect.dims().height()};
}

inline std::array<line, 4> lines(const rectangle& rect) {
    auto a1 = rect.corner();
    auto a2 = point{rect.corner().x(), int16_t(rect.corner().y() + rect.dims().height())};
    auto a3 = other_corner(rect);
    auto a4 = point{int16_t(rect.corner().x() + rect.dims().width()), rect.corner().y()};
    return {line{a1, a2}, line{a2, a3}, {a3, a4}, {a4, a1}};
}

inline float length(const line& l) {
    auto diff_x_2 = (l.p0().x() - l.p1().x()) * (l.p0().x() - l.p1().x());
    auto diff_y_2 = (l.p0().y() - l.p1().y()) * (l.p0().y() - l.p1().y());

    return std::sqrt(diff_x_2 + diff_y_2);
}

inline line shorten(const line& l, int radius) {
    auto ratio = radius / length(l);
    auto direction_x = (l.p1().x() - l.p0().x()) * ratio;
    auto direction_y = (l.p1().y() - l.p0().y()) * ratio;

    return line(point(l.p0().x() + direction_x, l.p0().y() + direction_y),
                point(l.p1().x() - direction_x, l.p1().y() - direction_y));
}
} // namespace tos::gfx2
