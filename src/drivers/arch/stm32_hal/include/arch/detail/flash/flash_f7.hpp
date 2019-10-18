#pragma once

#include <arch/detail/flash_common.hpp>

#include <stm32_hal/flash.hpp>
#include <stm32_hal/flash_ex.hpp>

namespace tos::stm32::detail {
inline namespace f7 {
class flash_ll {
public:
    using flash_align_t = uint64_t;

    void clear_flash_flags() {
        __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    }

    auto write_one_async(uint32_t addr, const flash_align_t& data) {
        return HAL_FLASH_Program_IT(FLASH_TYPEPROGRAM_DOUBLEWORD, addr, data);
    }

    static constexpr detail::flash_info info{4, 32768, 0x0800'0000};
};
} // namespace f7
} // namespace tos::stm32::detail