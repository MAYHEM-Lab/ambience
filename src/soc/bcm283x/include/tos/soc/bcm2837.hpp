#pragma once

#include <tos/soc/bcm283x.hpp>

namespace tos::bcm2837 {
using namespace tos::bcm283x;

constexpr auto IO_BASE = 0x3F000000;
constexpr auto IO_END = 0x3FFFFFFF;

constexpr auto GPU_IO_BASE = 0x7E000000;
constexpr auto GPU_MEM_BASE = 0xC0000000;

// Convert ARM address to GPU bus address
constexpr auto gpu_bus_address(uintptr_t addr) {
    return ((addr) & ~0xC0000000) | GPU_MEM_BASE;
}

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

constexpr auto EMMC_OFFSET = 0x300000;
constexpr auto EMMC_ADDRESS = IO_BASE + EMMC_OFFSET;

constexpr auto SDHOST_OFFSET = 0x202000;
constexpr auto SDHOST_ADDRESS = IO_BASE + SDHOST_OFFSET;

inline auto INTERRUPT_CONTROLLER =
    reinterpret_cast<volatile interrupt_controller_control_block*>(
        INTERRUPT_CONTROLLER_ADDRESS);
inline auto SYSTEM_TIMER =
    reinterpret_cast<volatile system_timer_control_block*>(SYSTEM_TIMER_ADDRESS);
inline auto UART0 = reinterpret_cast<volatile uart0_control_block*>(UART0_ADDRESS);
inline auto GPIO = reinterpret_cast<volatile gpio_control_block*>(GPIO_ADDRESS);
inline auto VIDEOCORE_MBOX =
    reinterpret_cast<volatile messagebox_control_block*>(VIDEOCORE_MBOX_ADDRESS);
inline auto EMMC = reinterpret_cast<volatile emmc_control_block*>(EMMC_ADDRESS);
inline auto SDHOST = reinterpret_cast<volatile sdhost_control_block*>(SDHOST_ADDRESS);
}