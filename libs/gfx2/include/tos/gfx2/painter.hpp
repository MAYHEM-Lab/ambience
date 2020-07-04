#pragma once

#include <painter_generated.hpp>
#include <tos/flags.hpp>
#include <tos/gfx2.hpp>

namespace tos::gfx2 {
enum class circle_quarters
{
    first = 1,
    second = 2,
    third = 4,
    fourth = 8,
    all = 15
};

template<circle_quarters quarters, class PointDrawer>
void draw_circle_quarter(const point& center,
                         const int8_t& org_radius,
                         const bool& fill,
                         const PointDrawer& draw_point) {
    auto radius = org_radius - 1;
    auto& [x0, y0] = center;
    int x = radius;
    int y = 0;
    int xChange = 1 - (radius << 1);
    int yChange = 0;
    int radiusError = 0;

    while (x >= y) {
        if (!fill) {
            if constexpr (util::is_flag_set(quarters, circle_quarters::first)) {
                draw_point({x0 + x, y0 - y}); // [0, 45]
                draw_point({x0 + y, y0 - x}); // [45, 90]
            }

            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::second)) {
                draw_point({x0 - y, y0 - x}); // [90, 135]
                draw_point({x0 - x, y0 - y}); // [135, 180]
            }

            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::third)) {
                draw_point({x0 - x, y0 + y}); // [180, 225]
                draw_point({x0 - y, y0 + x}); // [225, 270]
            }

            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::fourth)) {
                draw_point({x0 + y, y0 + x}); // [270, 315]
                draw_point({x0 + x, y0 + y}); // [315, 360]
            }
        } else {
            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::first)) {
                for (int i = 0; i <= x; ++i) {
                    draw_point({x0 + i, y0 - y}); // [0, 45]
                }
                for (int i = 0; i <= y; ++i) {
                    draw_point({x0 + i, y0 - x}); // [45, 90]
                }
            }
            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::second)) {
                for (int i = 0; i <= x; ++i) {
                    draw_point({x0 - y, y0 - i}); // [90, 135]
                }
                for (int i = 0; i <= y; ++i) {
                    draw_point({x0 - x, y0 - i}); // [135, 180]
                }
            }
            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::third)) {
                for (int i = 0; i <= x; ++i) {
                    draw_point({x0 - i, y0 + y}); // [180, 225]
                }
                for (int i = 0; i <= y; ++i) {
                    draw_point({x0 - i, y0 + x}); // [225, 270]
                }
            }
            if constexpr (tos::util::is_flag_set(quarters, circle_quarters::fourth)) {
                for (int i = 0; i <= x; ++i) {
                    draw_point({x0 + y, y0 + i}); // [270, 315]
                }
                for (int i = 0; i <= y; ++i) {
                    draw_point({x0 + x, y0 + i}); // [315, 360]
                }
            }
        }

        y++;
        radiusError += yChange;
        yChange += 2;
        if (((radiusError << 1) + xChange) > 0) {
            x--;
            radiusError += xChange;
            xChange += 2;
        }
    }
}

template<class PointDrawer>
void draw_rounder_rectangle_corners(const std::array<line, 4>& lines,
                                    int8_t radius,
                                    bool fill,
                                    const PointDrawer& draw_point) {
    auto top_right =
        tos::gfx2::point{lines[2].p1().x() - radius + 1, lines[2].p1().y() + radius - 1};
    auto bot_right =
        tos::gfx2::point{lines[1].p1().x() - radius + 1, lines[1].p1().y() - radius + 1};
    auto top_left =
        tos::gfx2::point{lines[3].p1().x() + radius - 1, lines[3].p1().y() + radius - 1};
    auto bot_left =
        tos::gfx2::point{lines[0].p1().x() + radius - 1, lines[0].p1().y() - radius + 1};

    draw_circle_quarter<tos::gfx2::circle_quarters::first>(
        top_right, radius, fill, draw_point);
    draw_circle_quarter<tos::gfx2::circle_quarters::fourth>(
        bot_right, radius, fill, draw_point);
    draw_circle_quarter<tos::gfx2::circle_quarters::second>(
        top_left, radius, fill, draw_point);
    draw_circle_quarter<tos::gfx2::circle_quarters::third>(
        bot_left, radius, fill, draw_point);
}
} // namespace tos::gfx2