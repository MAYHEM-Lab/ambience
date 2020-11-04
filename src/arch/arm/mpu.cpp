#include <tos/arm/cmsis.hpp>
#include <tos/arm/mpu.hpp>
#include <tos/arm/nvic.hpp>
#include <tos/barrier.hpp>
#include <tos/flags.hpp>
#include <tos/math/nearest_power_of_two.hpp>

#if defined(MPU)
namespace tos::arm {
namespace {
void nvic_set_priority(IRQn_Type irq, int preempt, int sub) {
    auto prioritygroup = NVIC_GetPriorityGrouping();
    NVIC_SetPriority(irq, NVIC_EncodePriority(prioritygroup, preempt, sub));
}
} // namespace
mpu::mpu()
    : tracked_driver(0) {
}

mpu::~mpu() {
    disable();
}

void mpu::enable() {
    // Enable MPU and let privileged mode access everything
    MPU->CTRL = MPU_CTRL_ENABLE_Msk | MPU_CTRL_PRIVDEFENA_Msk;

#if defined(SCB_SHCSR_MEMFAULTENA_Msk)
    // Enable the MemFault exception to be fired
    // Otherwise we'll get a HardFault
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;

    nvic_set_priority(MemoryManagement_IRQn, 0, 0);
    NVIC_EnableIRQ(MemoryManagement_IRQn);
#endif
}
void mpu::disable() {
    MPU->CTRL = 0;
    SCB->SHCSR &= ~SCB_SHCSR_MEMFAULTENA_Msk;
    NVIC_DisableIRQ(MemoryManagement_IRQn);
}

void mpu::enable_default_privileged_access() {
    MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk;
}
void mpu::disable_default_privileged_access() {
    MPU->CTRL &= ~MPU_CTRL_PRIVDEFENA_Msk;
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

    dmb();

    if (!(MPU->RASR & MPU_RASR_ENABLE_Msk)) {
        return std::nullopt;
    }

    uintptr_t base = MPU->RBAR;
    uint32_t size = 1 << (((MPU->RASR & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos) + 1);

    return tos::memory_region{/*.base =*/base, /*.size =*/size};
}

tos::expected<void, mpu_errors> mpu::set_region(int region_id,
                                                const tos::memory_region& region,
                                                permissions perms,
                                                bool shareable) {
    const uint32_t tmp_rbar = region.base | MPU_RBAR_VALID_Msk | region_id;

    // Actual size is computed as 2^(size field + 1)
    const auto size_field = math::nearest_power_of_two(region.size) - 1;

    uint8_t permissions = 0;
    if (util::is_flag_set(perms, permissions::read_write)) {
        permissions = 0b011;
    } else if (util::is_flag_set(perms, permissions::read)) {
        permissions = 0b110;
    } else if (util::is_flag_set(perms, permissions::write)) {
        return unexpected(mpu_errors::bad_permissions);
    }

    RASR_reg_t rasr{};
    rasr.cacheable = true;
    rasr.bufferable = false;
    rasr.shareable = shareable;
    rasr.execute_never = !util::is_flag_set(perms, permissions::execute);
    rasr.type_extension = 0;
    rasr.access_permissions = permissions;
    rasr.subregion_disable = 0;
    rasr.size = size_field;
    rasr.enable = true;

    MPU->RBAR = tmp_rbar;

    dmb();

    MPU->RASR = rasr.raw;

    dmb();

    return {};
}

void mpu::isr() {
    m_callback();
}
} // namespace tos::arm
#endif
