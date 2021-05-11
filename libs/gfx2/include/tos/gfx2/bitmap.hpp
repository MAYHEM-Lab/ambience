#pragma once

#include <tos/gfx2.hpp>

namespace tos::gfx2 {
class bitmap {
public:
    bitmap(color::alternatives color_type, span<uint8_t> buffer, gfx2::size dimensions, int stride)
        : m_color_type{color_type}
        , m_data{buffer}
        , m_dimensions{dimensions}
        , m_stride{stride} {
    }

    bitmap(color::alternatives color_type, span<uint8_t> buffer, gfx2::size dimensions)
        : bitmap(color_type, buffer, dimensions, dimensions.width()) {
    }

    color::alternatives color_type() const {
        return m_color_type;
    }

    template<class ColorType>
    tos::span<ColorType> get_data_as() {
        if constexpr (std::is_same_v<ColorType, rgb8>) {
            Assert(color_type() == color::alternatives::rgb);
            return {reinterpret_cast<rgb8*>(m_data.data()), m_data.size() / sizeof(rgb8)};
        }
        if constexpr (std::is_same_v<ColorType, mono8>) {
            Assert(color_type() == color::alternatives::mono);
            return {reinterpret_cast<mono8*>(m_data.data()),
                    m_data.size() / sizeof(mono8)};
        }
        if constexpr (std::is_same_v<ColorType, binary_color>) {
            Assert(color_type() == color::alternatives::binary);
            return {reinterpret_cast<binary_color*>(m_data.data()),
                    m_data.size() / sizeof(binary_color)};
        }
    }

    tos::span<uint8_t> get_data_raw() {
        return m_data;
    }

    int stride() const {
        return m_stride;
    }

    const size& dimensions() const {
        return m_dimensions;
    }

private:
    color::alternatives m_color_type;
    tos::span<uint8_t> m_data;
    size m_dimensions;
    int m_stride;
};
} // namespace tos::gfx2