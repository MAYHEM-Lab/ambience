#pragma once

#include <cstdint>
#include <memory>
#include <string_view>
#include <tos/gfx/bitmap.hpp>
#include <tos/gfx/color.hpp>
#include <tos/gfx/dimensions.hpp>
#include <tos/span.hpp>

namespace tos::gfx {
struct font_impl;
struct font {
public:
    static font make(span<const uint8_t> truetype_contents);

    [[nodiscard]] dimensions text_dimensions(std::string_view text,
                                             int text_height) const;

    [[nodiscard]] basic_bitmap_view<mono8> render_text(std::string_view text,
                                                       int text_height,
                                                       basic_bitmap_view<mono8> to) const;

    font(font&&) = default;

    ~font();

    [[nodiscard]] bool valid() const {
        return m_impl != nullptr;
    }

private:
    font() = default;
    std::unique_ptr<font_impl> m_impl;
};
} // namespace tos::gfx