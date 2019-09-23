//
// Created by fatih on 6/2/19.
//

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <tos/span.hpp>

namespace tos::gfx {
template<size_t W, size_t H>
class static_canvas_storage {
protected:
    constexpr tos::span<uint8_t> buffer() {
        return m_buf;
    }

    constexpr tos::span<const uint8_t> buffer() const {
        return m_buf;
    }

    constexpr size_t width() const {
        return W;
    }

    constexpr size_t height() const {
        return H;
    }

private:
    std::array<uint8_t, W * H / 8> m_buf;
};

class dynamic_canvas_storage {
public:
    dynamic_canvas_storage(uint16_t width, uint16_t height)
        : m_w{width}
        , m_h{height}
        , m_sz(width * height / 8)
        , m_buf(std::make_unique<uint8_t[]>(m_sz)) {
    }

protected:
    tos::span<uint8_t> buffer() {
        return {m_buf.get(), m_sz};
    }

    tos::span<const uint8_t> buffer() const {
        return {m_buf.get(), m_sz};
    }

    size_t width() const {
        return m_w;
    }
    size_t height() const {
        return m_h;
    }

private:
    uint16_t m_w, m_h;
    size_t m_sz;
    std::unique_ptr<uint8_t[]> m_buf;
};

template<class StorageT>
class basic_canvas : private StorageT {
    using StorageT::buffer;

public:
    using StorageT::StorageT;

    constexpr void set_pixel(size_t x, size_t y, bool val) {
        auto word_idx = (width() * y + x) / 8;
        auto word_off = (width() * y + x) % 8;

        auto& word = buffer()[word_idx];

        if (val) {
            word |= (0x80 >> word_off);
        } else {
            word &= ~(0x80 >> word_off);
        }
    }

    constexpr void set_word(size_t x, size_t y, uint8_t val) {
        auto word_idx = (width() * y + x) / 8;
        auto& word = buffer()[word_idx];
        word = val;
    }

    constexpr void fill(bool val) {
        uint8_t v = val ? 0xFF : 0;
        for (auto& x : buffer()) {
            x = v;
        }
    }

    constexpr tos::span<const uint8_t> data() const {
        return buffer();
    }

    using StorageT::height;
    using StorageT::width;

    constexpr bool get_pixel(size_t x, size_t y) const {
        auto word_off = (width() * y + x) % 8;
        auto word = get_word(x, y);
        return word & (0x80 >> word_off);
    }

    constexpr uint8_t get_word(size_t x, size_t y) const {
        auto word_idx = (width() * y + x) / 8;
        auto& word = buffer()[word_idx];
        return word;
    }
};

template<size_t W, size_t H>
using fixed_canvas = basic_canvas<static_canvas_storage<W, H>>;

using dynamic_canvas = basic_canvas<dynamic_canvas_storage>;

template<class InCanvasT, class OutCanvasT>
constexpr void copy(const InCanvasT& src, OutCanvasT& out, size_t x, size_t y) {
    for (size_t i = 0; i < src.height(); ++i) {
        for (size_t j = 0; j < src.width(); ++j) {
            out.set_pixel(x + j, y + i, src.get_pixel(j, i));
        }
    }
}

template<class InCanvasT, class OutCanvasT>
constexpr auto copy(const InCanvasT& src, OutCanvasT& out) {
    const auto x_scale = float(src.width()) / out.width();
    const auto y_scale = float(src.height()) / out.height();

    for (int i = 0; i < out.height(); ++i) {
        for (int j = 0; j < out.width(); ++j) {
            out.set_pixel(j, i, src.get_pixel(j * x_scale, i * y_scale));
        }
    }
}

template<size_t ToW, size_t ToH, class FromCanvas>
constexpr auto upscale(const FromCanvas& src) -> fixed_canvas<ToW, ToH> {
    fixed_canvas<ToW, ToH> res{};
    copy(src, res);
    return res;
}

template<class CanvasT, class FontT>
constexpr void
draw_text(CanvasT& framebuf, const FontT& font, tos::span<const char> str, int x, int y) {
    for (char c : str) {
        if (c == 0)
            return;
        auto ch = font.get(c);
        if (!ch) {
            ch = font.get('?');
        }
        auto scaled = *ch;
        using tos::gfx::copy;
        copy(scaled, framebuf, x, y);
        y += scaled.height();
    }
}
} // namespace tos::gfx