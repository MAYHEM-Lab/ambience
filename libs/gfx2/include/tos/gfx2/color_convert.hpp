#pragma once

#include <tos/gfx2.hpp>
#include <type_traits>

namespace tos::gfx2 {
template<class ToColor,
         class FromColor,
         std::enable_if_t<!std::is_same_v<ToColor, FromColor>>* = nullptr>
ToColor color_convert(const FromColor& from);

template<class Color>
Color color_convert(const Color& color) {
    return color;
}

template<>
mono8 color_convert(const rgb8& from);

template<>
binary_color color_convert(const mono8& from);

template <>
rgb8 color_convert(const mono8& from);

template <>
mono8 color_convert(const binary_color& from);

template<>
inline binary_color color_convert(const rgb8& from) {
    return color_convert<binary_color>(color_convert<mono8>(from));
}

template<>
inline rgb8 color_convert(const binary_color& from) {
    return color_convert<rgb8>(color_convert<mono8>(from));
}
} // namespace tos::gfx2