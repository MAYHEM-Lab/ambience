#pragma once

#include <cstdint>
#include <tos/function_ref.hpp>

namespace tos::virtio {
struct transport {
    virtual uint8_t read_byte(int offset) = 0;
    virtual void write_byte(int offset, uint8_t data) = 0;

    virtual uint16_t read_u16(int offset) = 0;
    virtual void write_u16(int offset, uint16_t data) = 0;

    virtual uint32_t read_u32(int offset) = 0;
    virtual void write_u32(int offset, uint32_t data) = 0;

    virtual void enable_interrupts(tos::function_ref<void()> handler) = 0;
    virtual void disable_interrupts() = 0;

    virtual ~transport() = default;
};
} // namespace tos::virtio