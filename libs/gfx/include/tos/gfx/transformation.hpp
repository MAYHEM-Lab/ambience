//
// Created by fatih on 9/20/19.
//

#pragma once

#include "canvas.hpp"

namespace tos::gfx {
/**
 * Given two canvases, inverts the colors of the first canvas to the
 * out canvas.
 *
 * Expects the out canvas to be greater or equal in size to the input
 * canvas.
 */
template<class InCanvasT, class OutCanvasT>
constexpr void invert(const InCanvasT& in, OutCanvasT& out) {
    for (size_t i = 0; i < in.height(); ++i) {
        for (size_t j = 0; j < in.width(); ++j) {
            out.set_pixel(j, i, !in.get_pixel(j, i));
        }
    }
}

/**
 * Mirrors the input canvas to the output canvas in the X dimension.
 *
 * Expects the out canvas to be greater or equal in size to the input
 * canvas.
 */
template<size_t W, size_t H>
constexpr void mirror_horizontal(const fixed_canvas<W, H>& in, fixed_canvas<W, H>& out) {
    for (size_t i = 0; i < in.height(); ++i) {
        for (size_t j = 0; j < in.width(); ++j) {
            out.set_pixel(j, i, in.get_pixel(in.width() - j - 1, i));
        }
    }
}

/**
 * Transposes the input canvas to the output canvas. Transposing means assigning
 * every (i, j)th pixel in the input canvas to (j, i)th pixel in the output canvas.
 */
template<size_t W, size_t H>
constexpr void transpose(const fixed_canvas<W, H>& in, fixed_canvas<H, W>& out) {
    for (size_t i = 0; i < out.height(); ++i) {
        for (size_t j = 0; j < out.width(); ++j) {
            out.set_pixel(j, i, in.get_pixel(i, j));
        }
    }
}

template<class InCanvasT, class OutCanvasT>
constexpr void copy(const InCanvasT& src, OutCanvasT& out, point p) {
    for (size_t i = 0; i < src.height(); ++i) {
        for (size_t j = 0; j < src.width(); ++j) {
            out.set_pixel(p.x + j, p.y + i, src.get_pixel(j, i));
        }
    }
}

/**
 * Given two canvases, copies the pixels of the source canvas to the output
 * canvas while up/downsampling as necessary.
 */
template<class InCanvasT, class OutCanvasT>
constexpr auto copy(const InCanvasT& src, OutCanvasT& out) {
    const auto x_scale = float(src.width()) / out.width();
    const auto y_scale = float(src.height()) / out.height();

    for (size_t i = 0; i < out.height(); ++i) {
        for (size_t j = 0; j < out.width(); ++j) {
            out.set_pixel(j, i, src.get_pixel(j * x_scale, i * y_scale));
        }
    }
}

template<size_t W, size_t H>
constexpr fixed_canvas<H, W> transpose_copy(const fixed_canvas<W, H>& in) {
    fixed_canvas<H, W> out{};
    transpose(in, out);
    return out;
}

template<size_t W, size_t H>
constexpr fixed_canvas<W, H> mirror_horizontal_copy(const fixed_canvas<W, H>& in) {
    fixed_canvas<W, H> out{};
    mirror_horizontal(in, out);
    return out;
}
} // namespace tos::gfx