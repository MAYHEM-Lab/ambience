#include <tos/debug/log.hpp>
#include <tos/gfx/text.hpp>
#include <tos/gfx2/bit_painter.hpp>
#include <tos/gfx2/color_convert.hpp>
#include <tos/gfx2/utility.hpp>

namespace tos::gfx2 {
bit_painter::bit_painter(tos::span<uint8_t> buffer, const size& dims)
    : m_fb{buffer}
    , m_dims{dims}
    , m_col(binary_color{true}) {
}

int8_t
bit_painter::draw_rect(const rectangle& rect, const int8_t& radius, const bool& fill) {
    auto draw_point = [this](const tos::gfx2::point& pt) { this->draw_point(pt); };

    if (fill) {
        if (radius == 0) {
            if (rect.corner() == tos::gfx2::point{0, 0} &&
                rect.dims() == get_dimensions()) {
                this->fill();
                return 0;
            }
            for (int16_t row = rect.corner().y(); row <= other_corner(rect).y(); ++row) {
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

int8_t bit_painter::draw_text(std::string_view text, const point& p) {
    static constexpr auto font = tos::gfx::basic_font().mirror_horizontal();
    LOG("Drawing text", text);
    draw_text_line(text, font, p);
    return 0;
}

template<class FontT>
void bit_painter::draw_text_line(std::string_view str,
                                 const FontT& font,
                                 tos::gfx2::point p) {
    LOG("Drawing text line", str);
    for (char c : str) {
        if (c == 0)
            return;
        auto ch = font.get(c);
        if (!ch) {
            return;
        }
        auto scaled = *ch;
        using tos::gfx::copy;

        for (size_t i = 0; i < scaled.height(); ++i) {
            for (size_t j = 0; j < scaled.width(); ++j) {
                if (scaled.get_pixel(j, i)) {
                    draw_point({p.x() + j, p.y() + i});
                }
            }
        }

        p.x() += scaled.width();
    }
}

void bit_painter::draw_line(const point& p0, const point& p1) {
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
        draw(bit_location(index));
        const int errorIsTooBig = error >= dLong;
        index += offset[errorIsTooBig];
        error += abs_d[errorIsTooBig];
    }
}

int8_t bit_painter::set_style(const services::style& s) {
    m_col = visit([](auto& col) { return color_convert<binary_color>(col); }, s.color());
    return 0;
}
} // namespace tos::gfx2