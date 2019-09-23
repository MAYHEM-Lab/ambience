//
// Created by fatih on 6/2/19.
//

#pragma once

#include "canvas.hpp"
#include "dimensions.hpp"
#include "transformation.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <frozen/algorithm.h>
#include <frozen/unordered_map.h>
#include <type_traits>

namespace tos::gfx {
template<size_t W, size_t H>
struct basic_character : public fixed_canvas<W, H> {
public:
    basic_character() = default;
    explicit constexpr basic_character(const std::array<uint8_t, 8>& buf) {
        for (size_t i = 0; i < buf.size(); ++i) {
            this->set_word(0, i, buf[i]);
        }
    }

    constexpr basic_character inverted() const {
        basic_character out{{}};
        invert(*this, out);
        return out;
    }

    constexpr basic_character mirror_horizontal() const {
        basic_character mirrored_out{{}};
        tos::gfx::mirror_horizontal(*this, mirrored_out);
        return mirrored_out;
    }

    constexpr basic_character rotate_90_cw() const {
        basic_character out{{}};
        transpose(*this, out);
        basic_character mirrored_out{{}};
        tos::gfx::mirror_horizontal(out, mirrored_out);
        return mirrored_out;
    }

    constexpr basic_character transposed() const {
        basic_character out{{}};
        transpose(*this, out);
        return out;
    }

    template<size_t NewW, size_t NewH>
    constexpr basic_character<NewW, NewH> resize() const {
        basic_character<NewW, NewH> out{{}};
        copy(*this, out);
        return out;
    }
};

template<class Fn, typename K, typename V, size_t N, size_t... Ns>
constexpr auto
map(Fn&& fn, frozen::unordered_map<K, V, N> const& map0, std::index_sequence<Ns...>) {
    auto iter0 = map0.begin();
    using RetT = std::remove_const_t<std::remove_reference_t<decltype(fn(*iter0))>>;
    RetT x[N] = {((void)Ns, fn(*iter0++))...};
    frozen::bits::carray<std::pair<K, typename RetT::second_type>, N> out(x);
    return frozen::unordered_map<K, typename RetT::second_type, N>{out};
}

template<class Fn, typename K, typename V, size_t N, size_t... Ns>
constexpr auto map(Fn&& fn, frozen::unordered_map<K, V, N> const& map0) {
    return map(fn, map0, std::make_index_sequence<N>());
}

template<size_t Len, class CharT = basic_character<8, 8>>
class charset {
public:
    constexpr charset(frozen::unordered_map<char, CharT, Len> chars)
        : m_chars{chars} {
    }

    constexpr const CharT* get(char c) const {
        auto it = m_chars.find(c);
        if (it == m_chars.end())
            return nullptr;
        return &it->second;
    }

    template<class RetT = charset, class FunT>
    constexpr auto transform(FunT&& fn) const {
        return RetT{
            map([&](auto x) { return std::pair(x.first, (x.second.*fn)()); }, m_chars)};
    }

    constexpr auto inverted() const {
        return transform(&CharT::inverted);
    }

    constexpr auto mirror_horizontal() const {
        return transform(&CharT::mirror_horizontal);
    }

    constexpr auto transposed() const {
        return transform(&CharT::transposed);
    }

    constexpr auto rotate_90_cw() const {
        return transform(&CharT::rotate_90_cw);
    }

    template<size_t NewW, size_t NewH>
    constexpr auto resize() const {
        return transform<charset<Len, basic_character<NewW, NewH>>>(
            &CharT::template resize<NewW, NewH>);
    }

private:
    frozen::unordered_map<char, CharT, Len> m_chars;
};

constexpr auto basic_font() {
    using character_type = basic_character<8, 8>;
    constexpr character_type A{{0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33}};
    constexpr character_type B{{0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F}};
    constexpr character_type C{{0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C}};
    constexpr character_type D{{0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F}};
    constexpr character_type E{{0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F}};
    constexpr character_type F{{0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F}};
    constexpr character_type G{{0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C}};
    constexpr character_type H{{0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33}};
    constexpr character_type I{{0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E}};
    constexpr character_type J{{0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}};
    constexpr character_type K{{0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67}};
    constexpr character_type L{{0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F}};
    constexpr character_type M{{0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63}};
    constexpr character_type N{{0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63}};
    constexpr character_type O{{0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C}};
    constexpr character_type P{{0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F}};
    constexpr character_type Q{{0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38}};
    constexpr character_type R{{0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67}};
    constexpr character_type S{{0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E}};
    constexpr character_type T{{0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E}};
    constexpr character_type U{{0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F}};
    constexpr character_type V{{0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C}};
    constexpr character_type W{{0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63}};
    constexpr character_type X{{0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63}};
    constexpr character_type Y{{0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E}};
    constexpr character_type Z{{0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F}};
    constexpr character_type a{{0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}};
    constexpr character_type b{{0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}};
    constexpr character_type c{{0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}};
    constexpr character_type d{{0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}};
    constexpr character_type e{{0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}};
    constexpr character_type f{{0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}};
    constexpr character_type g{{0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}};
    constexpr character_type h{{0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}};
    constexpr character_type i{{0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}};
    constexpr character_type j{{0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}};
    constexpr character_type k{{0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}};
    constexpr character_type l{{0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}};
    constexpr character_type m{{0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}};
    constexpr character_type n{{0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}};
    constexpr character_type o{{0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}};
    constexpr character_type p{{0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}};
    constexpr character_type q{{0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}};
    constexpr character_type r{{0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}};
    constexpr character_type s{{0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}};
    constexpr character_type t{{0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}};
    constexpr character_type u{{0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}};
    constexpr character_type v{{0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}};
    constexpr character_type w{{0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}};
    constexpr character_type x{{0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}};
    constexpr character_type y{{0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}};
    constexpr character_type z{{0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}};
    constexpr character_type space{{0, 0, 0, 0, 0, 0, 0, 0}};
    constexpr character_type question{{0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C}};
    constexpr character_type dot{{0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}};

    constexpr character_type c0{{0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}};
    constexpr character_type c1{{0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}};
    constexpr character_type c2{{0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}};
    constexpr character_type c3{{0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}};
    constexpr character_type c4{{0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}};
    constexpr character_type c5{{0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}};
    constexpr character_type c6{{0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}};
    constexpr character_type c7{{0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}};
    constexpr character_type c8{{0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}};
    constexpr character_type c9{{0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}};

    constexpr character_type colon{{0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}};
    constexpr character_type dash{{0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}};
    constexpr character_type slash{{0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}};

    constexpr character_type tick{{0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}};

    return charset<69>(frozen::make_unordered_map<char, character_type>(
        {{'A', A},    {'B', B},     {'C', C},    {'D', D},  {'E', E},     {'F', F},
         {'G', G},    {'H', H},     {'I', I},    {'J', J},  {'K', K},     {'L', L},
         {'M', M},    {'N', N},     {'O', O},    {'P', P},  {'Q', Q},     {'R', R},
         {'S', S},    {'T', T},     {'U', U},    {'V', V},  {'W', W},     {'X', X},
         {'Y', Y},    {'Z', Z},     {'a', a},    {'b', b},  {'c', c},     {'d', d},
         {'e', e},    {'f', f},     {'g', g},    {'h', h},  {'i', i},     {'j', j},
         {'k', k},    {'l', l},     {'m', m},    {'n', n},  {'o', o},     {'p', p},
         {'q', q},    {'r', r},     {'s', s},    {'t', t},  {'u', u},     {'v', v},
         {'w', w},    {'x', x},     {'y', y},    {'z', z},  {' ', space}, {'?', question},
         {'.', dot},  {'0', c0},    {'1', c1},   {'2', c2}, {'3', c3},    {'4', c4},
         {'5', c5},   {'6', c6},    {'7', c7},   {'8', c8}, {'9', c9},    {':', colon},
         {'-', dash}, {'/', slash}, {'\'', tick}}));
}

enum class text_direction
{
    vertical,
    horizontal
};

/**
 * Draws the given text with the given font on the given canvas at the specified
 * coordinates. This function does not handle new lines and only renders horizontally.
 * Upon an un-renderable character (such as a new line, tab etc.), the function will
 * return early.
 */
template<class CanvasT, class FontT>
constexpr void draw_text_line(CanvasT& canvas,
                              const FontT& font,
                              tos::span<const char> str,
                              point p,
                              text_direction direction = text_direction::horizontal) {
    for (char c : str) {
        if (c == 0)
            return;
        auto ch = font.get(c);
        if (!ch) {
            return;
        }
        auto scaled = *ch;
        using tos::gfx::copy;
        copy(scaled, canvas, p);
        if (direction == text_direction::horizontal) {
            p.x += scaled.width();
        } else {
            p.y += scaled.height();
        }
    }
}

template<class CanvasT, class FontT>
constexpr void draw_text(CanvasT& canvas,
                         const FontT& font,
                         tos::span<const char> text,
                         point p,
                         text_direction direction = text_direction::horizontal) {
    auto row_begin = text.begin();
    do {
        auto row_end = std::find(row_begin, text.end(), '\n');
        draw_text_line(canvas,
                       font,
                       tos::span<const char>(row_begin, row_end),
                       // TODO: this should take the newlines into account
                       p,
                       direction);
        row_begin = row_end;
    } while (row_begin != text.end());
}

/**
 * Given the text and the font, computes the dimensions the text would use on a
 * canvas.
 * The dimensions are computed as if the text is rendered left to right, in a
 * horizontal manner.
 */
template<class FontT>
constexpr dimensions text_dimensions(const FontT& font, std::string_view text) {
    int rows = 0;
    int max_cols = 0;
    int cur_cols = 0;

    for (char c : text) {
        if (c == '\n') {
            ++rows;
            max_cols = max_cols < cur_cols ? cur_cols : max_cols;
            cur_cols = 0;
            continue;
        }
        cur_cols += font.get(c)->width();
    }

    if (rows == 0 && cur_cols != 0) {
        rows = 1;
        max_cols = cur_cols;
    }

    return {max_cols, rows * font.get('0')->height()};
}

template<class FontT>
dynamic_canvas text_to_canvas(const FontT& font,
                              std::string_view text,
                              text_direction direction = text_direction::horizontal) {
    auto dims = text_dimensions(font, text);
    if (direction == text_direction::vertical) {
        std::swap(dims.height, dims.width);
    }
    dynamic_canvas res(dims);
    draw_text(res,
              font,
              tos::span<const char>(text.begin(), text.end()),
              point{0, 0},
              direction);
    return res;
}
} // namespace tos::gfx
