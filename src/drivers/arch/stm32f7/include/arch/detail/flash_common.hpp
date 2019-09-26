#pragma once

#include <cstdint>

namespace tos::stm32::detail {
struct flash_info {
    uint16_t num_pages;
    uint16_t page_size;
    uintptr_t base_addr;
};
} // namespace tos::stm32::detail