#pragma once

#include "tos/gfx/dimensions.hpp"

#include <tos/span.hpp>

namespace tos::gfx {
template<class ColorT>
class basic_bitmap_view {
public:
    using color_type = ColorT;

    basic_bitmap_view(span<ColorT> data, int stride, const dimensions& dims)
        : m_buffer{data}
        , m_stride{stride}
        , m_dims{dims} {
    }

    const ColorT* data() const {
        return m_buffer.data();
    }

    ColorT* data() {
        return m_buffer.data();
    }

    ColorT& at(const point& p) {
        auto offset = p.x + p.y * stride();
        return m_buffer[offset];
    }

    ColorT& at(int row, int col) {
        return at({col, row});
    }

    const ColorT& at(const point& p) const {
        auto offset = p.x + p.y * stride();
        return m_buffer[offset];
    }

    const ColorT& at(int row, int col) const {
        return at({col, row});
    }

    basic_bitmap_view slice(const rectangle& rect) {
        auto offset = rect.corner.x + rect.corner.y * stride();
        return basic_bitmap_view(m_buffer.slice(offset), m_stride, rect.size);
    }

    basic_bitmap_view slice(const point& at, const dimensions& dims) {
        return slice(rectangle{at, dims});
    }

    [[nodiscard]] int stride() const {
        return m_stride;
    }

    [[nodiscard]] const dimensions& dims() const {
        return m_dims;
    }

    span<ColorT> row(int row) {
        auto offset = row * stride();
        return m_buffer.slice(offset, dims().width);
    }

    span<const ColorT> row(int row) const {
        auto offset = row * stride();
        return m_buffer.slice(offset, dims().width);
    }

    span<ColorT> raw_data() {
        return m_buffer;
    }

private:
    span<ColorT> m_buffer;
    int m_stride;
    dimensions m_dims;
};
} // namespace tos::gfx