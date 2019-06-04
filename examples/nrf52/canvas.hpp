//
// Created by fatih on 6/2/19.
//

#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/span.hpp>
#include <array>

namespace bakir
{
template <size_t W, size_t H>
class canvas
{
public:
    constexpr void set_pixel(size_t x, size_t y, bool val) {
        auto word_idx = (W * y + x) / 8;
        auto word_off = (W * y + x) % 8;

        auto& word = m_buf[word_idx];

        if (val)
        {
            word |= (0x80 >> word_off);
        }
        else
        {
            word &= ~(0x80 >> word_off);
        }
    }

    constexpr void set_word(size_t x, size_t y, uint8_t val) {
        auto word_idx = (W * y + x) / 8;
        auto& word = m_buf[word_idx];
        word = val;
    }

    template <class FromT>
    constexpr void copy(FromT&& from, size_t x, size_t y)
    {
        /*
         * if the target pixel is a multiple of 8 and the from size is a multiple of 8, take the fast path
         */
        if ((x % 8) == 0 && (from.width() % 8) == 0)
        {
            for (int i = 0; i < from.height(); ++i)
            {
                for (int j = 0; j < from.width() / 8; ++j)
                {
                    set_word(x + j, y + i, from.get_word(j, i));
                }
            }
        }
        else
        {
            for (int i = 0; i < from.height(); ++i)
            {
                for (int j = 0; j < from.width(); ++j)
                {
                    set_pixel(x + j, y + i, from.get_pixel(j, i));
                }
            }
        }
    }

    constexpr void fill(bool val) {
        uint8_t v = val ? 0xFF : 0;
        m_buf.fill(v);
    }

    constexpr tos::span<const uint8_t> data() const {
        return m_buf;
    }

    constexpr size_t width() const { return W; }
    constexpr size_t height() const { return H; }

    constexpr bool get_pixel(size_t x, size_t y) const {
        auto word_idx = (W * y + x) / 8;
        auto word_off = (W * y + x) % 8;
        auto& word = m_buf[word_idx];
        return word & (0x80 >> word_off);
    }

    constexpr uint8_t get_word(size_t x, size_t y) const {
        auto word_idx = (W * y + x) / 8;
        auto& word = m_buf[word_idx];
        return word;
    }

private:
    std::array<uint8_t, W * H / 8> m_buf;
};
}