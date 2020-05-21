//
// Created by fatih on 4/21/20.
//

#include "stb_truetype.h"
#include "tos/gfx/dimensions.hpp"

#include <cstdint>
#include <tos/debug/assert.hpp>
#include <tos/debug/log.hpp>
#include <tos/gfx/truetype.hpp>
#include <tos/span.hpp>

namespace tos::gfx {
struct font_impl {
    stbtt_fontinfo m_font;
};

namespace {
std::unique_ptr<font_impl> open(span<const uint8_t> truetype_contents) {
    auto f = std::make_unique<font_impl>();
    if (!f) {
        return f;
    }

    auto init_res =
        stbtt_InitFont(&f->m_font,
                       truetype_contents.data(),
                       stbtt_GetFontOffsetForIndex(truetype_contents.data(), 0));
    Assert(init_res != 0);
    return f;
}
} // namespace

font font::make(span<const uint8_t> truetype_contents) {
    font f;
    f.m_impl = open(truetype_contents);
    return f;
}

font::~font() = default;
// int font::text_width(std::string_view text) const {
//     float scale = stbtt_ScaleForPixelHeight(&m_impl->m_font, 16);
//     int total_width = 0;
//     for (auto c : text) {
//         total_width += codepoint_width(c);
//     }
//     return total_width * scale;
// }

// int font::codepoint_width(int32_t codepoint) const {
//     int advance_width, left_side_bearing;
//     stbtt_GetCodepointHMetrics(
//         &m_impl->m_font, codepoint, &advance_width, &left_side_bearing);
//     return advance_width;
// }

basic_bitmap_view<mono8> font::render_text(std::string_view text,
                                           int text_height,
                                           basic_bitmap_view<mono8> to) const {
    float scale = stbtt_ScaleForPixelHeight(&m_impl->m_font, text_height);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_impl->m_font, &ascent, &descent, &lineGap);

    ascent *= scale;
    descent *= scale;

    int x = 0;
    int max_y = 0;
    for (auto it = text.begin(); it != text.end(); ++it) {
        auto c = *it;
        /* how wide is this character */
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&m_impl->m_font, c, &ax, &lsb);

        /* get bounding box for character (may be offset to account for chars that dip
         * above or below the line */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(
            &m_impl->m_font, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

        /* compute y (different characters have different heights */
        int y = ascent + c_y1;

        auto window = to.slice(point{int16_t(x + (lsb * scale)), int16_t(y)},
                               dimensions{c_x2 - c_x1, c_y2 - c_y1});

        max_y = std::max<int>(y + c_y2 - c_y1, max_y);
        Assert(max_y < to.dims().height);

        stbtt_MakeCodepointBitmap(&m_impl->m_font,
                                  reinterpret_cast<uint8_t*>(window.data()),
                                  c_x2 - c_x1,
                                  c_y2 - c_y1,
                                  to.stride(),
                                  scale,
                                  scale,
                                  c);

        /* advance x */
        x += ax * scale;

        if (it + 1 != text.end()) {
            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(&m_impl->m_font, c, *(it + 1));
            x += kern * scale;
        }
    }

    return to.slice({0, 0}, {x, text_height});
}

dimensions font::text_dimensions(std::string_view text, int text_height) const {
    float scale = stbtt_ScaleForPixelHeight(&m_impl->m_font, text_height);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_impl->m_font, &ascent, &descent, &lineGap);

    ascent *= scale;
    descent *= scale;

    int x = 0;
    int max_y = 0;
    for (auto it = text.begin(); it != text.end(); ++it) {
        auto c = *it;
        /* how wide is this character */
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&m_impl->m_font, c, &ax, &lsb);

        /* get bounding box for character (may be offset to account for chars that dip
         * above or below the line */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(
            &m_impl->m_font, c, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);

        /* compute y (different characters have different heights */
        int y = ascent + c_y1;

        max_y = std::max<int>(y + c_y2 - c_y1, max_y);
        
        /* advance x */
        x += ax * scale;

        if (it + 1 != text.end()) {
            /* add kerning */
            int kern;
            kern = stbtt_GetCodepointKernAdvance(&m_impl->m_font, c, *(it + 1));
            x += kern * scale;
        }
    }

    return {x, max_y};
}

font::font(font&&) = default;
font& font::operator=(font&&) = default;
} // namespace tos::gfx