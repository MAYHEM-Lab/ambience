#pragma once

#include <cstdint>

namespace tos::x86_64 {
struct port {
    constexpr port(uint16_t port_addr)
        : m_port{port_addr} {
    }

    void outb(uint8_t b) {
        asm volatile("outb %0, %1" : : "a"(b), "Nd"(m_port));
    }

    inline uint8_t inb() {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(m_port));
        return ret;
    }

    void outw(uint16_t w) {
        asm volatile("outw %0, %1" : : "a"(w), "Nd"(m_port));
    }

    void outl(uint32_t l) {
        asm volatile("outl %0, %1" : : "a"(l), "Nd"(m_port));
    }

private:
    uint16_t m_port;
};
} // namespace tos::x86_64