#pragma once

#include "flash_common.hpp"

#include <stm32_hal/flash.hpp>
#include <stm32_hal/flash_ex.hpp>

namespace tos::stm32::detail {
inline namespace f1 {
class flash_ll {
public:
    using flash_align_t = uint32_t;

    inline void clear_flash_flags() {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_BSY |
                               FLASH_FLAG_OPTVERR);     // Clear of every FLASH flag
    }

    auto write_one_async(uint32_t addr, const flash_align_t& data) {
        return HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_WORD, addr, data);
    }

    static constexpr detail::flash_info info{128, 1024, 0x0800'0000};
};
} // namespace l0
} // namespace tos::stm32::detail