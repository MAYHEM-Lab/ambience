#pragma once

#include <cstdint>
#include <tos/gfx/dimensions.hpp>
#include <tos/span.hpp>

namespace tos::raspi3 {
class framebuffer {
public:
    explicit framebuffer(gfx::dimensions dims);

    gfx::dimensions dims() const {
        return m_physical;
    }

    span<uint8_t> get_buffer() const {
        return m_buffer;
    }

private:
    gfx::dimensions m_physical;
    span<uint8_t> m_buffer{nullptr};
};
}