//
// Created by fatih on 6/2/19.
//

#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <frozen/unordered_map.h>
#include <frozen/algorithm.h>
#include <type_traits>

namespace bakir
{
struct character
{
public:
    constexpr bool get_pixel(size_t x, size_t y) const {
        return m_buf[y] & (0x80 >> x);
    }

    constexpr uint8_t get_word(size_t, size_t y) const {
        return m_buf[y];
    }

    constexpr uint8_t width() const { return 8; }
    constexpr uint8_t height() const { return 8; }

    constexpr character inverted() const {
        std::array<uint8_t, 8> res{};
        for (int i = 0; i < m_buf.size(); ++i)
        {
            res[i] = ~m_buf[i];
        }
        return {res};
    }

    constexpr character flip_horizontal() const {
        std::array<uint8_t, 8> res{};
        for (int i = 0; i < m_buf.size(); ++i)
        {
            auto b = m_buf[i];
            b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
            b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
            b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
            res[i] = b;
        }
        return {res};
    }

    constexpr character transpose() const {
        std::array<uint8_t, 8> res{};
        for (int i = 0; i < m_buf.size(); ++i)
        {
            uint8_t line{};
            for (int j = 0; j < m_buf.size(); ++j)
            {
            }
        }
        return {res};
    }

    std::array<uint8_t, 8> m_buf; // 8x8
};

constexpr auto get_def_chars()
{
    constexpr character A {{ 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}};
    constexpr character B {{ 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}};
    constexpr character C {{ 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}};

    return frozen::unordered_map<char, character, 3>{
        { 'A', A },
        { 'B', B },
        { 'C', C }
    };
}

template <class Fn, typename K, typename V, size_t N, size_t... Ns>
constexpr auto map(Fn&& fn, frozen::unordered_map<K, V, N> const &map0,
                          std::index_sequence<Ns...>)
{
    auto iter0 = map0.begin();
    using RetT = std::remove_const_t <std::remove_reference_t<decltype(*iter0)>>;
    RetT x[N] = { ((void)Ns, fn(*iter0++))... };
    frozen::bits::carray<std::pair<K, typename RetT::second_type>, N> out(x);
    return frozen::unordered_map<K, typename RetT::second_type, N>{out};
}

template <class Fn, typename K, typename V, size_t N, size_t... Ns>
constexpr auto map(Fn&& fn, frozen::unordered_map<K, V, N> const &map0) {
    return map(fn, map0, std::make_index_sequence<N>());
}

class charset
{
public:
    constexpr charset() : charset{get_def_chars()} {}

    constexpr const character* get(char c) const
    {
        auto it = m_chars.find(c);
        if (it == m_chars.end()) return nullptr;
        return &it->second;
    }

    constexpr auto inverted() const {
        return charset{ map([](auto x) { return std::pair<char, character>(x.first, x.second.inverted()); }, m_chars) };
    }

    constexpr auto flip_horizontal() const {
        return charset{ map([](auto x) { return std::pair<char, character>(x.first, x.second.flip_horizontal()); }, m_chars) };
    }

private:
    constexpr charset(frozen::unordered_map<char, character, 3> chars) : m_chars{chars} {}

    frozen::unordered_map<char, character, 3> m_chars;
};

} // namespace bakir