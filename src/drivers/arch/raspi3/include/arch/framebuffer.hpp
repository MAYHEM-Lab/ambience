#pragma once

#include <cstdint>
#include <tos/gfx/dimensions.hpp>
#include <tos/span.hpp>
#include <tos/self_pointing.hpp>

namespace tos::raspi3 {
class framebuffer : public self_pointing<framebuffer> {
public:
    explicit framebuffer(gfx::dimensions dims);

    gfx::dimensions dims() const {
        return m_physical;
    }

    span<uint8_t> get_buffer() const {
        return m_buffer;
    }

    void set_pixel(gfx::point pt, bool val) {
        auto pos = (pt.y * m_physical.width + pt.x) * 3;
        get_buffer()[pos] = val ? 255 : 0;
        get_buffer()[pos + 1] = val ? 255 : 0;
        get_buffer()[pos + 2] = val ? 255 : 0;
    }

    void set_pixel(int x, int y, bool val) {
        set_pixel({static_cast<uint16_t>(x), uint16_t(y)}, val);
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
    span<uint8_t> m_buffer{nullptr};
};
}