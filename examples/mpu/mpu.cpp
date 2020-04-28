#include "mpu.hpp"

#include <tos/barrier.hpp>

extern "C" void MemManage_Handler() {
    tos::cmsis::mpu::get(0)->isr();
}

namespace tos::cmsis {
mpu::mpu()
    : tracked_driver(0) {
    // Enable MPU and let privileged mode access everything
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;

    // Enable the MemFault exception to be fired
    // Otherwise we'll get a HardFault
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    NVIC_EnableIRQ(MemoryManagement_IRQn);
}

size_t mpu::min_region_size() const {
    MPU->RBAR = 0xffffffe0;
    tos::detail::memory_barrier();
    const auto rbar = MPU->RBAR;
    return ~rbar + 1;
}

size_t mpu::num_supported_regions() const {
    uint32_t type_reg = MPU->TYPE;
    return (type_reg & MPU_TYPE_DREGION_Msk) >> MPU_TYPE_DREGION_Pos;
}

std::optional<tos::memory_region> mpu::get_region(int region_id) {
    MPU->RNR = region_id;
    tos::detail::memory_barrier();

    if (!(MPU->RBAR & MPU_RBAR_VALID_Msk)) {
        return std::nullopt;
    }

    uintptr_t base = MPU->RBAR >> MPU_RBAR_ADDR_Pos;
    uint32_t size = 1 << (((MPU->RASR & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos) + 1);

    return tos::memory_region{/*.base =*/base, /*.size =*/size};
}

tos::expected<void, mpu_errors> mpu::set_region(int region_id,
                                                const tos::memory_region& region) {
    MPU->RNR = region_id;
    tos::detail::memory_barrier();
    constexpr uint32_t mask = 64 - 1;

    // the base address is aligned to 64 bytes!
    const uint32_t tmp_rbar =
        ((region.base + mask) & ~mask) | MPU_RBAR_VALID_Msk | region_id;

    auto nearest_power_of_two = [](uint32_t size) {
      int i = 0;
      uint32_t accum = 1;
      for (; accum < size; ++i, accum <<= 1)
          ;
      return i;
    };

    // Actual size is computed as 2^(size field + 1)
    const auto size_field = nearest_power_of_two(region.size) - 1;

    RASR_reg_t rasr{};
    rasr.execute_never = 1;
    rasr.cacheable = 1;
    rasr.bufferable = 0;
    rasr.shareable = 1;
    rasr.type_extension = 0;
    rasr.access_permissions = 0;
    rasr.subregion_disable = 0;
    rasr.size = size_field;
    rasr.enable = true;

    MPU->RBAR = tmp_rbar;

    tos::detail::memory_barrier();

    MPU->RASR = rasr.raw;

    return {};
}

void mpu::isr() {
    m_callback();
}
} // namespace cmsis
