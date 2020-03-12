#pragma once

#include <cstddef>
#include <cstdint>

namespace bcm2837 {
constexpr auto IO_BASE = 0x3F000000;

constexpr auto GPU_IO_BASE = 0x7E000000;
constexpr auto GPU_MEM_BASE = 0xC0000000;

// Convert ARM address to GPU bus address
constexpr auto gpu_bus_address(uintptr_t addr) {
    return ((addr) & ~0xC0000000) | GPU_MEM_BASE;
}

struct uart0_control_block {
    uint32_t DR;
    uint32_t RSRECR;
    uint8_t __pad__[16];
    uint32_t FR;
    uint8_t __pad_[4];
    uint32_t ILPR;
    uint32_t IBRD;
    uint32_t FBRD;
    uint32_t LCRH;
    uint32_t CR;
    uint32_t IFLS;
    uint32_t IMSC;
    uint32_t RIS;
    uint32_t MIS;
    uint32_t ICR;
    uint32_t DMACR;
    uint32_t ITCR;
    uint32_t ITIP;
    uint32_t ITOP;
    uint32_t TDR;
};

struct gpio_control_block {
    uint8_t __pad__[148];
    uint32_t GPPUD;
    uint32_t GPPUDCLK0;
};

struct messagebox_control_block {

    uint32_t read;
    uint8_t __pad__[12];
    uint32_t poll;
    uint32_t sender;
    uint32_t status;
    uint32_t config;
    uint32_t write;

    [[nodiscard]]
    bool status_empty() const volatile {
        return status & (1U << 30U);
    }

    [[nodiscard]]
    bool status_full() const volatile {
        return status & (1U << 31U);
    }
};

constexpr auto UART0_OFFSET = 0x201000;
constexpr auto UART0_ADDRESS = IO_BASE + UART0_OFFSET;

constexpr auto GPIO_OFFSET = 0x200000;
constexpr auto GPIO_ADDRESS = IO_BASE + GPIO_OFFSET;

constexpr auto VIDEOCORE_MBOX_OFFSET = 0xB880;
constexpr auto VIDEOCORE_MBOX_ADDRESS = IO_BASE + VIDEOCORE_MBOX_OFFSET;

inline auto UART0 = reinterpret_cast<volatile uart0_control_block*>(UART0_ADDRESS);
inline auto GPIO = reinterpret_cast<volatile gpio_control_block*>(GPIO_ADDRESS);
inline auto VIDEOCORE_MBOX =
    reinterpret_cast<volatile messagebox_control_block*>(VIDEOCORE_MBOX_ADDRESS);
} // namespace bcm2837