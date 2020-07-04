#pragma once

#include "utility.hpp"

#include <tos/gfx2/color_convert.hpp>
#include <tos/gfx2/painter.hpp>
#include <tos/span.hpp>

namespace tos::gfx2 {
class buffer_painter : public services::painter {
    using ColorT = rgb8;

public:
    buffer_painter(tos::span<ColorT> buffer, const size& dims)
        : m_fb{buffer}
        , m_dims{dims}
        , m_col{255, 255, 255} {
    }

    int8_t draw_point(const tos::gfx2::point& p) override {
        auto idx = p.y() * m_dims.width() + p.x();
        m_fb[idx] = m_col;
        return 0;
    }

    int8_t set_style(const services::style& s) override {
        m_col = visit([](auto& col) { return color_convert<ColorT>(col); }, s.color());
        return 0;
    }

    size get_dimensions() override {
        return m_dims;
    }

    int8_t draw_line(const line& l) override {
        draw_line(l.p0(), l.p1());
        return 0;
    }

    void fill() {
        std::fill(m_fb.begin(), m_fb.end(), m_col);
    }

    int8_t
    draw_rect(const rectangle& rect, const int8_t& radius, const bool& fill) override {
        auto draw_point = [this](const tos::gfx2::point& pt) { this->draw_point(pt); };

        if (fill) {
            if (radius == 0) {
                if (rect.corner() == tos::gfx2::point{0, 0} &&
                    rect.dims() == get_dimensions()) {
                    this->fill();
                    return 0;
                }
                for (int16_t row = rect.corner().y(); row <= other_corner(rect).y();
                     ++row) {
                    for (int16_t col = rect.corner().x(); col <= other_corner(rect).x();
                         ++col) {
                        draw_point(tos::gfx2::point{col, row});
                    }
                }
                return 0;
            }

            // rounded corners with fill
            auto lines = tos::gfx2::lines(rect);

            draw_rounder_rectangle_corners(lines, radius, fill, draw_point);

            auto big_rect = tos::gfx2::rectangle{
                {lines[3].p1().x() + radius, lines[3].p1().y()},
                {rect.dims().width() - 2 * radius, rect.dims().height()}};
            draw_rect(big_rect, 0, fill);

            auto left_rect =
                tos::gfx2::rectangle{{lines[3].p1().x(), lines[3].p1().y() + radius},
                                     {radius, rect.dims().height() - 2 * radius}};
            draw_rect(left_rect, 0, fill);

            auto right_rect = tos::gfx2::rectangle{
                {lines[2].p1().x() - radius + 1, lines[2].p1().y() + radius},
                {radius, rect.dims().height() - 2 * radius}};
            draw_rect(right_rect, 0, fill);

            return 0;
        }

        if (radius == 0) {
            for (auto& line : lines(rect)) {
                draw_line(line);
            }
            return 0;
        }

        // rounded corners without fill
        auto lines = tos::gfx2::lines(rect);
        draw_rounder_rectangle_corners(lines, radius, fill, draw_point);

        for (auto& line : lines) {
            auto shorter_line = shorten(line, radius);
            draw_line(shorter_line);
        }

        return 0;
    }

    int8_t draw_circle(const tos::gfx2::point& center,
                       const int8_t& radius,
                       const bool& fill) override {
        draw_circle_quarter<tos::gfx2::circle_quarters::all>(center, radius, fill);
        return 0;
    }

    int8_t draw_text(std::string_view text, const point& p) override {
        return 0;
    }

    int8_t flush() override {
        return 0;
    }

private:
    void draw(int index) {
        m_fb[index] = m_col;
    }

    void draw_line(const point& p0, const point& p1) {
        int dx = p1.x() - p0.x();
        int dy = p1.y() - p0.y();

        int dLong = abs(dx);
        int dShort = abs(dy);

        int offsetLong = dx > 0 ? 1 : -1;
        int offsetShort = dy > 0 ? m_dims.width() : -m_dims.width();

        if (dLong < dShort) {
            using std::swap;
            swap(dShort, dLong);
            swap(offsetShort, offsetLong);
        }

        int error = dLong / 2;
        int index = p0.y() * m_dims.width() + p0.x();
        const int offset[] = {offsetLong, offsetLong + offsetShort};
        const int abs_d[] = {dShort, dShort - dLong};
        for (int i = 0; i <= dLong; ++i) {
            draw(index);
            const int errorIsTooBig = error >= dLong;
            index += offset[errorIsTooBig];
            error += abs_d[errorIsTooBig];
        }
    }


    template<tos::gfx2::circle_quarters quarters>
    void
    draw_circle_quarter(const point& center, const int8_t& radius, const bool& fill) {
        tos::gfx2::draw_circle_quarter<quarters>(
            center, radius, fill, [this](const tos::gfx2::point& pt) {
                this->draw_point(pt);
            });
    }

    tos::span<ColorT> m_fb;
    tos::gfx2::size m_dims;
    ColorT m_col;
};
} // namespace tos::gfx2