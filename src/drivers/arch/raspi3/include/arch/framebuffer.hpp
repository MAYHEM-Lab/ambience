#pragma once

#include <cstdint>
#include <tos/debug/log.hpp>
#include <tos/gfx/dimensions.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>


namespace tos::raspi3 {
class framebuffer : public self_pointing<framebuffer> {
public:
    explicit framebuffer(gfx::dimensions dims);

    gfx::dimensions virtual_dims() const {
        return m_virtual;
    }

    gfx::dimensions dims() const {
        return m_physical;
    }

    span<uint8_t> get_buffer();

    void swap_buffers();

    void set_pixel(const gfx::point& pt, bool val) {
        auto pos = (pt.y * m_physical.width + pt.x) * 3;
        get_buffer()[pos] = val ? 255 : 0;
        get_buffer()[pos + 1] = val ? 255 : 0;
        get_buffer()[pos + 2] = val ? 255 : 0;
    }

    void set_pixel(int x, int y, bool val) {
        set_pixel({static_cast<int16_t>(x), int16_t(y)}, val);
    }

    int get_pitch() const {
        return m_pitch;
    }

    bool is_rgb() const {
        return m_rgb;
    }

    int bit_depth() const {
        return m_bit_depth;
    }

private:
    int32_t m_bit_depth;
    bool m_rgb;
    int32_t m_pitch;
    gfx::dimensions m_physical;
    gfx::dimensions m_virtual;
    span<uint8_t> m_buffer{nullptr};
    bool m_swapped = false;
};
} // namespace tos::raspi3