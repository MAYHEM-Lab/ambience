#pragma once

#include <cstdint>
#include <tos/gfx2.hpp>
#include <tos/gfx2/painter.hpp>
#include <tos/span.hpp>

namespace tos::gfx2 {
class bit_painter : public services::painter {
public:
    bit_painter(tos::span<uint8_t> buffer, const size& dims);

    int8_t draw_circle(const tos::gfx2::point& center,
                       const int8_t& radius,
                       const bool& fill) override {
        draw_circle_quarter<tos::gfx2::circle_quarters::all>(center, radius, fill);
        return 0;
    }

    void draw_line(const tos::gfx2::point& p0, const tos::gfx2::point& p1);

    int8_t draw_line(const tos::gfx2::line& l) override {
        auto& [p0, p1] = l;
        draw_line(p0, p1);
        return 0;
    }

    int8_t draw_point(const tos::gfx2::point& p) override {
        auto loc = bit_location(p);
        draw(loc);
        return 0;
    }

    int8_t draw_rect(const tos::gfx2::rectangle& rect,
                     const int8_t& radius,
                     const bool& fill) override;

    int8_t set_style(const tos::services::style& s) override {
        m_style = s;
        return 0;
    }

    tos::gfx2::size get_dimensions() override {
        return m_dims;
    }

    void fill() {
        auto fbyte = m_style.color().binary().col() ? 0xFF : 0;
        fill_byte(fbyte);
    }

    void fill_byte(uint8_t fbyte) {
        std::fill(m_fb.begin(), m_fb.end(), fbyte);
    }

    void draw(const tos::gfx2::point& p) {
        auto loc = bit_location(p);
        draw(loc);
    }

    int8_t draw_text(std::string_view text, const tos::gfx2::point& p) override;

    void flush() {
    }

private:
    template<tos::gfx2::circle_quarters quarters>
    void draw_circle_quarter(const tos::gfx2::point& center,
                             const int8_t& radius,
                             const bool& fill);

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
        auto absolute_bit_pos = p.y() * m_dims.width() + p.x();
        return bit_location(absolute_bit_pos);
    }

    void draw(const bit_loc& loc) {
        auto byte = read_byte(loc.byte_num);
        if (m_style.color().binary().col()) {
            byte |= 1 << loc.bit_pos;
        } else {
            byte &= ~(1 << loc.bit_pos);
        }
        write_byte(loc.byte_num, byte);
    }

    [[nodiscard]] constexpr bit_loc bit_location(int bitpos) const {
        auto byte_num = bitpos / 8;
        auto bit_pos = bitpos % 8;
        return {byte_num, 7 - bit_pos};
    }

    tos::span<uint8_t> m_fb;
    tos::gfx2::size m_dims;
    tos::services::style m_style;
};
} // namespace tos::gfx2