//
// Created by fatih on 10/24/19.
//

#pragma once

#include <cstdint>
#include <tos/print.hpp>

namespace tos::gfx {
template<class RedT, class GreenT = RedT, class BlueT = RedT>
struct basic_rgb {
    RedT red;
    GreenT green;
    BlueT blue;
};

template<class HueT, class SaturationT, class ValueT>
struct basic_hsv {
    HueT hue;
    SaturationT saturation;
    ValueT value;
};


struct monochrome {
    uint8_t bw;
};

using mono8 = monochrome;
using rgb8 = basic_rgb<uint8_t>;
using hsv8 = basic_hsv<uint16_t, uint8_t, uint8_t>;

template<class RedT, class GreenT, class BlueT, class RatioT>
basic_rgb<RedT, GreenT, BlueT> lerp(const basic_rgb<RedT, GreenT, BlueT>& from,
                                    const basic_rgb<RedT, GreenT, BlueT>& to,
                                    const RatioT& parameter) {
    return {
        static_cast<RedT>(from.red + (to.red - from.red) * parameter),
        static_cast<GreenT>(from.green + (to.green - from.green) * parameter),
        static_cast<BlueT>(from.blue + (to.blue - from.blue) * parameter),
    };
}

template <class StreamT, class RedT, class GreenT, class BlueT>
void print(StreamT& stream, const basic_rgb<RedT, GreenT, BlueT>& color) {
    tos::print(stream, "rgb{", int(color.red), ',', int(color.green), ',', int(color.blue), "}");
}
} // namespace tos::gfx