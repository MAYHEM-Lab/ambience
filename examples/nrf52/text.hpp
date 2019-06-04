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
#include "canvas.hpp"

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

    constexpr character mirror_horizontal() const {
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

    constexpr character rotate_90_cw() const {
        return transposed().mirror_horizontal();
    }

    constexpr character transposed() const {
        canvas<8, 8> self{};
        for (int row = 0; row < 8; ++row)
        {
            self.set_word(0, row, m_buf[row]);
        }

        canvas<8, 8> out{};

        for (int i = 0; i < out.height(); ++i)
        {
            for (int j = 0; j < out.width(); ++j)
            {
                out.set_pixel(j, i, self.get_pixel(i, j));
            }
        }

        std::array<uint8_t, 8> res{};
        for (int row = 0; row < 8; ++row)
        {
            res[row] = out.get_word(0, row);
        }
        return {res};
    }

    std::array<uint8_t, 8> m_buf; // 8x8
};

template <class Fn, typename K, typename V, size_t N, size_t... Ns>
constexpr auto map(Fn&& fn, frozen::unordered_map<K, V, N> const &map0, std::index_sequence<Ns...>)
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

template <size_t Len>
class charset
{
public:
    constexpr charset(frozen::unordered_map<char, character, Len> chars) : m_chars{chars} {}

    constexpr const character* get(char c) const
    {
        auto it = m_chars.find(c);
        if (it == m_chars.end()) return nullptr;
        return &it->second;
    }

    template <class FunT>
    constexpr auto transform(FunT&& fn) const {
        return charset{ map([&](auto x) { return std::pair<char, character>(x.first, (x.second.*fn)()); }, m_chars) };
    }

    constexpr auto inverted() const {
        return transform(&character::inverted);
    }

    constexpr auto mirror_horizontal() const {
        return transform(&character::mirror_horizontal);
    }

    constexpr auto transposed() const {
        return transform(&character::transposed);
    }

    constexpr auto rotate_90_cw() const {
        return transform(&character::rotate_90_cw);
    }

private:
    frozen::unordered_map<char, character, Len> m_chars;
};

constexpr auto basic_font()
{
    constexpr character A {{ 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33}};
    constexpr character B {{ 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F}};
    constexpr character C {{ 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C}};
    constexpr character D {{ 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F}};
    constexpr character E {{ 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F}};
    constexpr character F {{ 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F}};
    constexpr character G {{ 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C}};
    constexpr character H {{ 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33}};
    constexpr character I {{ 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E}};
    constexpr character J {{ 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}};
    constexpr character K {{ 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67}};
    constexpr character L {{ 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F}};
    constexpr character M {{ 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63}};
    constexpr character N {{ 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63}};
    constexpr character O {{ 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C}};
    constexpr character P {{ 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F}};
    constexpr character Q {{ 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38}};
    constexpr character R {{ 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67}};
    constexpr character S {{ 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E}};
    constexpr character T {{ 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E}};
    constexpr character U {{ 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F}};
    constexpr character V {{ 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C}};
    constexpr character W {{ 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63}};
    constexpr character X {{ 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63}};
    constexpr character Y {{ 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E}};
    constexpr character Z {{ 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F}};
    constexpr character a {{ 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}};
    constexpr character b {{ 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}};
    constexpr character c {{ 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}};
    constexpr character d {{ 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}};
    constexpr character e {{ 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}};
    constexpr character f {{ 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}};
    constexpr character g {{ 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}};
    constexpr character h {{ 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}};
    constexpr character i {{ 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}};
    constexpr character j {{ 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}};
    constexpr character k {{ 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}};
    constexpr character l {{ 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}};
    constexpr character m {{ 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}};
    constexpr character n {{ 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}};
    constexpr character o {{ 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}};
    constexpr character p {{ 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}};
    constexpr character q {{ 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}};
    constexpr character r {{ 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}};
    constexpr character s {{ 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}};
    constexpr character t {{ 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}};
    constexpr character u {{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}};
    constexpr character v {{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}};
    constexpr character w {{ 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}};
    constexpr character x {{ 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}};
    constexpr character y {{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}};
    constexpr character z {{ 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}};
    constexpr character space {{0,0,0,0,0,0,0,0}};
    constexpr character question {{ 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C} };
    constexpr character dot {{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}};

    return charset<55>(frozen::make_unordered_map<char, character>({
        { 'A', A },
        { 'B', B },
        { 'C', C },
        { 'D', D },
        { 'E', E },
        { 'F', F },
        { 'G', G },
        { 'H', H },
        { 'I', I },
        { 'J', J },
        { 'K', K },
        { 'L', L },
        { 'M', M },
        { 'N', N },
        { 'O', O },
        { 'P', P },
        { 'Q', Q },
        { 'R', R },
        { 'S', S },
        { 'T', T },
        { 'U', U },
        { 'V', V },
        { 'W', W },
        { 'X', X },
        { 'Y', Y },
        { 'Z', Z },
        { 'a', a },
        { 'b', b },
        { 'c', c },
        { 'd', d },
        { 'e', e },
        { 'f', f },
        { 'g', g },
        { 'h', h },
        { 'i', i },
        { 'j', j },
        { 'k', k },
        { 'l', l },
        { 'm', m },
        { 'n', n },
        { 'o', o },
        { 'p', p },
        { 'q', q },
        { 'r', r },
        { 's', s },
        { 't', t },
        { 'u', u },
        { 'v', v },
        { 'w', w },
        { 'x', x },
        { 'y', y },
        { 'z', z },
        { ' ', space },
        { '?', question },
        { '.', dot }
    }));
}
} // namespace bakir