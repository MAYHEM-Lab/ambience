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

constexpr auto UART0_OFFSET = 0x201000;
constexpr auto UART0_ADDRESS = IO_BASE + UART0_OFFSET;

struct gpio_control_block {
    uint8_t __pad__[148];
    uint32_t GPPUD;
    uint32_t GPPUDCLK0;
};

constexpr auto GPIO_OFFSET = 0x200000;
constexpr auto GPIO_ADDRESS = IO_BASE + GPIO_OFFSET;

inline auto UART0 = reinterpret_cast<volatile uart0_control_block*>(UART0_ADDRESS);
inline auto GPIO = reinterpret_cast<volatile gpio_control_block*>(GPIO_ADDRESS);
}