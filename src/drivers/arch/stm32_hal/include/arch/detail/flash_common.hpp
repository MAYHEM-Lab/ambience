#pragma once

#include <cstdint>

namespace tos::stm32::detail {
struct flash_info {
    uint16_t num_pages;
    uint16_t page_size;

    // The address in memory where this flash is mapped to.
    uintptr_t base_addr;
};
} // namespace tos::stm32::detail