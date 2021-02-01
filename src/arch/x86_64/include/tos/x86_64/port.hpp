#pragma once

#include <cstdint>
#include <tos/barrier.hpp>

namespace tos::x86_64 {
struct port {
    constexpr port(uint16_t port_addr)
        : m_port{port_addr} {
    }

    void outb(uint8_t b) const {
        tos::detail::memory_barrier();
        asm volatile("outb %0, %1" : : "a"(b), "Nd"(m_port));
        tos::detail::memory_barrier();
    }

    inline uint8_t inb() const {
        uint8_t ret;
        tos::detail::memory_barrier();
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(m_port));
        tos::detail::memory_barrier();
        return ret;
    }

    void outw(uint16_t w) const {
        tos::detail::memory_barrier();
        asm volatile("outw %0, %1" : : "a"(w), "Nd"(m_port));
        tos::detail::memory_barrier();
    }

    inline uint16_t inw() const {
        uint16_t ret;
        tos::detail::memory_barrier();
        asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(m_port));
        tos::detail::memory_barrier();
        return ret;
    }

    void outl(uint32_t l) const {
        tos::detail::memory_barrier();
        asm volatile("outl %0, %1" : : "a"(l), "Nd"(m_port));
        tos::detail::memory_barrier();
    }

    inline uint32_t inl() const {
        uint32_t ret;
        tos::detail::memory_barrier();
        asm volatile("inl %1, %0" : "=a"(ret) : "Nd"(m_port));
        tos::detail::memory_barrier();
        return ret;
    }

private:
    uint16_t m_port;
};
} // namespace tos::x86_64