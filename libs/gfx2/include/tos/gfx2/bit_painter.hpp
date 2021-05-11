#pragma once

#include <cstdint>
#include <tos/gfx2.hpp>
#include <tos/gfx2/painter.hpp>
#include <tos/span.hpp>

namespace tos::gfx2 {
class bit_painter : public services::painter::sync_server {
public:
    bit_painter(tos::span<uint8_t> buffer, const size& dims);

    int8_t draw_circle(const tos::gfx2::point& center,
                       int8_t radius,
                       bool fill) override {
        this->draw_circle_quarter<tos::gfx2::circle_quarters::all>(center, radius, fill);
        return 0;
    }
    bool set_orientation(tos::services::rotation orientation) override;
    void draw_line(const tos::gfx2::point& p0, const tos::gfx2::point& p1);

    int8_t draw_line(const tos::gfx2::line& l) override {
        auto& [p0, p1] = l;
        draw_line(p0, p1);
        return 0;
    }

    int8_t draw_point(const tos::gfx2::point& org_p) override {
        auto p = translate_point(org_p);
        if (p.x() < 0 || p.y() < 0 || p.x() >= m_physical_dims.width() ||
            p.y() >= m_physical_dims.height()) {
            return 0;
        }

        draw_point_fast(p);
        return 0;
    }

    void draw_point_fast(const tos::gfx2::point& translated_checked_point) {
        auto loc = bit_location(translated_checked_point);
        draw(loc);
    }

    int8_t draw_rect(const tos::gfx2::rectangle& rect,
                     int8_t radius,
                     bool fill) override;

    int8_t set_style(const tos::services::style& s) override;

    tos::gfx2::size get_dimensions() override {
        return m_virtual_dims;
    }

    void fill() {
        auto fbyte = m_col.col() ? 0xFF : 0;
        fill_byte(fbyte);
    }

    void fill_byte(uint8_t fbyte) {
        std::fill(m_fb.begin(), m_fb.end(), fbyte);
    }

    int8_t draw_text(std::string_view text, const tos::gfx2::point& p) override;

    int8_t flush() override {
        return 0;
    }

    bool draw_bitmap(tos::gfx2::color::alternatives color_type,
                     tos::span<uint8_t> buffer,
                     int16_t stride,
                     const tos::gfx2::rectangle& image_rect,
                     const tos::gfx2::rectangle& screen_rect) override;
    
private:
    template<tos::gfx2::circle_quarters quarters>
    void
    draw_circle_quarter(const point& center, const int8_t& radius, const bool& fill) {
        tos::gfx2::draw_circle_quarter<quarters>(
            center, radius, fill, [this](const tos::gfx2::point& pt) {
                this->draw_point(pt);
            });
    }

    template<class FontT>
    void draw_text_line(std::string_view str, const FontT& font, tos::gfx2::point p);

    struct bit_loc {
        int byte_num;
        int bit_pos;
    };

    uint8_t read_byte(int num) {
        return m_fb[num];
    }

    void write_byte(int num, uint8_t byte) {
        m_fb[num] = byte;
    }

    [[nodiscard]] bit_loc bit_location(const tos::gfx2::point& p) const {
        auto absolute_bit_pos = p.y() * m_physical_dims.width() + p.x();
        return bit_location(absolute_bit_pos);
    }

    void draw(const bit_loc& loc, const gfx2::binary_color& col) {
        auto byte = read_byte(loc.byte_num);
        if (col.col()) {
            byte |= 1 << loc.bit_pos;
        } else {
            byte &= ~(1 << loc.bit_pos);
        }
        write_byte(loc.byte_num, byte);
    }

    void draw(const bit_loc& loc) {
        draw(loc, m_col);
    }

    [[nodiscard]] constexpr bit_loc bit_location(int bitpos) const {
        auto byte_num = bitpos / 8;
        auto bit_pos = bitpos % 8;
        return {byte_num, 7 - bit_pos};
    }

    gfx2::point translate_point(point p) {
        if (m_rotation == services::rotation::horizontal) {
            auto temp_y = p.y();
            p.y() = p.x();
            p.x() = m_physical_dims.width() - 1 - temp_y;
        }
        return p;
    }

    services::rotation m_rotation = services::rotation::vertical;
    tos::span<uint8_t> m_fb;
    tos::gfx2::size m_physical_dims;
    tos::gfx2::size m_virtual_dims;
    tos::gfx2::binary_color m_col;
};
} // namespace tos::gfx2