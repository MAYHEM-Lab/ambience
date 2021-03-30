#pragma once

#include <cmath>
#include <tos/gfx2.hpp>

namespace tos::gfx2 {
inline point other_corner(const rectangle& rect) {
    return {rect.corner().x() + rect.dims().width() - 1,
            rect.corner().y() + rect.dims().height() - 1};
}

inline std::array<line, 4> lines(const rectangle& rect) {
    auto a1 = rect.corner();
    auto a2 =
        point{rect.corner().x(), int16_t(rect.corner().y() + rect.dims().height() - 1)};
    auto a3 = other_corner(rect);
    auto a4 =
        point{int16_t(rect.corner().x() + rect.dims().width() - 1), rect.corner().y()};
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

inline rectangle intersection(const rectangle& a, const rectangle& b) {
    rectangle r{{0, 0}, {0, 0}};
    r.corner() = point{std::max(a.corner().x(), b.corner().x()),
                       std::max(a.corner().y(), b.corner().y())};
    auto other_corn = point{std::min(other_corner(a).x(), other_corner(b).x()),
                            std::min(other_corner(a).y(), other_corner(b).y())};
    r.dims().width() = other_corn.x() - r.corner().x() + 1;
    r.dims().height() = other_corn.y() - r.corner().y() + 1;
    return r;
}

inline rectangle bound(const rectangle& a, const rectangle& b) {
    rectangle r{{0, 0}, {0, 0}};
    r.corner() = point{std::min(a.corner().x(), b.corner().x()),
                       std::min(a.corner().y(), b.corner().y())};
    auto other_corn = point{std::max(other_corner(a).x(), other_corner(b).x()),
                            std::max(other_corner(a).y(), other_corner(b).y())};
    r.dims().width() = other_corn.x() - r.corner().x() + 1;
    r.dims().height() = other_corn.y() - r.corner().y() + 1;
    return r;
}
} // namespace tos::gfx2
