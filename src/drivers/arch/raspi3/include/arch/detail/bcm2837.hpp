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
    uint32_t GPFSEL0;
    uint32_t GPFSEL1;
    uint8_t pad[144];
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

    [[nodiscard]] bool status_empty() const volatile {
        return status & (1U << 30U);
    }

    [[nodiscard]] bool status_full() const volatile {
        return status & (1U << 31U);
    }
};

struct interrupt_controller_control_block {
    uint8_t __pad__[0x200];
    uint32_t irq_basic_pending;
    uint32_t irq_pending_1;
    uint32_t irq_pending_2;

    uint32_t fiq_control;

    uint32_t enable_irq_1;
    uint32_t enable_irq_2;
    uint32_t enable_basic_irq;

    uint32_t disable_irq_1;
    uint32_t disable_irq_2;
    uint32_t disable_basic_irq;
};

struct system_timer_control_block {
    uint32_t control_status;
    uint32_t counter_lo;
    uint32_t counter_hi;
    uint32_t compare0;
    uint32_t compare1;
    uint32_t compare2;
    uint32_t compare3;
};

constexpr auto INTERRUPT_CONTROLLER_OFFSET = 0xB000;
constexpr auto INTERRUPT_CONTROLLER_ADDRESS = IO_BASE + INTERRUPT_CONTROLLER_OFFSET;

constexpr auto SYSTEM_TIMER_OFFSET = 0x3000;
constexpr auto SYSTEM_TIMER_ADDRESS = IO_BASE + SYSTEM_TIMER_OFFSET;

constexpr auto UART0_OFFSET = 0x201000;
constexpr auto UART0_ADDRESS = IO_BASE + UART0_OFFSET;

constexpr auto GPIO_OFFSET = 0x200000;
constexpr auto GPIO_ADDRESS = IO_BASE + GPIO_OFFSET;

constexpr auto VIDEOCORE_MBOX_OFFSET = 0xB880;
constexpr auto VIDEOCORE_MBOX_ADDRESS = IO_BASE + VIDEOCORE_MBOX_OFFSET;

inline auto INTERRUPT_CONTROLLER =
    reinterpret_cast<volatile interrupt_controller_control_block*>(
        INTERRUPT_CONTROLLER_ADDRESS);
inline auto SYSTEM_TIMER =
    reinterpret_cast<volatile system_timer_control_block*>(SYSTEM_TIMER_ADDRESS);
inline auto UART0 = reinterpret_cast<volatile uart0_control_block*>(UART0_ADDRESS);
inline auto GPIO = reinterpret_cast<volatile gpio_control_block*>(GPIO_ADDRESS);
inline auto VIDEOCORE_MBOX =
    reinterpret_cast<volatile messagebox_control_block*>(VIDEOCORE_MBOX_ADDRESS);
} // namespace bcm2837