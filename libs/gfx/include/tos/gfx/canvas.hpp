//
// Created by fatih on 6/2/19.
//

#pragma once

#include "dimensions.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
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
    dynamic_canvas_storage(const dimensions& dims)
        : dynamic_canvas_storage(dims.width, dims.height) {
    }
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
        for (auto& word : buffer()) {
            word = v;
        }
    }

    constexpr tos::span<const uint8_t> data() const {
        return buffer();
    }

    using StorageT::height;
    using StorageT::width;

    constexpr dimensions dims() {
        return {width(), height()};
    }

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
} // namespace tos::gfx