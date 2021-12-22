#include <tos/debug/log.hpp>
#include <tos/gfx2/color_convert.hpp>

namespace tos::gfx2 {
template<>
mono8 color_convert(const rgb8& from) {
    return {0.2125 * from.red() + 0.7154 * from.green() + 0.0721 * from.blue()};
}

template<>
binary_color color_convert(const mono8& from) {
    return {from.col() > 128};
}

template<>
rgb8 color_convert(const mono8& from) {
    return {from.col(), from.col(), from.col()};
}

template <>
rgb8 color_convert(const argb8& from) {
    return rgb8{from.red(), from.green(), from.blue()};
}

template<>
mono8 color_convert(const binary_color& from) {
    return {from.col() ? 255 : 0};
}
} // namespace tos::gfx2