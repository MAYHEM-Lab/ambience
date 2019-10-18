//
// Created by fatih on 10/17/19.
//

#include "tos/debug/debug.hpp"
#include "tos/expected.hpp"
#include "tos/thread.hpp"

#include <arch/drivers.hpp>
#include <optional>
#include <tos/barrier.hpp>
#include <tos/ft.hpp>
#include <tos/memory.hpp>

namespace cmsis {
enum class mpu_errors
{};

class mpu
    : public tos::self_pointing<mpu>
    , public tos::tracked_driver<mpu, 1> {
public:
    mpu();

    size_t num_supported_regions() const;
    size_t min_region_size() const;

    std::optional<tos::memory_region> get_region(int region_id);
    tos::expected<void, mpu_errors> set_region(int region_id,
                                               const tos::memory_region& region);

    void isr();

private:
};
} // namespace cmsis

extern "C" void MemManage_Handler() {
    cmsis::mpu::get(0)->isr();
}

namespace cmsis {
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

    const uint32_t tmp_rasr =
        1 << MPU_RASR_XN_Pos | 1 << MPU_RASR_C_Pos | 0 << MPU_RASR_B_Pos |
        1 << MPU_RASR_S_Pos | 0b000 << MPU_RASR_TEX_Pos | 0b000 << MPU_RASR_AP_Pos |
        0 << MPU_RASR_SRD_Pos | size_field << MPU_RASR_SIZE_Pos | MPU_RASR_ENABLE_Msk;

    MPU->RBAR = tmp_rbar;

    tos::detail::memory_barrier();

    MPU->RASR = tmp_rasr;

    return {};
}

void mpu::isr() {
    __BKPT(0);
}
} // namespace cmsis

char foo[256];

void require_impl(bool expr, const char* /* str */) {
    if (!expr) {
        __BKPT(0);
    }
}

#define REQUIRE(expr) require_impl(bool(expr), #expr)

void mpu_task() {
    cmsis::mpu mpu;
    auto regs = mpu.num_supported_regions();
    tos::debug::do_not_optimize(&regs);

    auto min_size = mpu.min_region_size();
    tos::debug::do_not_optimize(&min_size);

    REQUIRE(!mpu.get_region(0));

    auto set_res =
        mpu.set_region(0, {.base = reinterpret_cast<uintptr_t>(&foo), .size = 128});
    REQUIRE(set_res);

    tos::debug::do_not_optimize(foo[128] = 5);
    tos::debug::do_not_optimize(foo[0] = 5);

    tos::this_thread::block_forever();
}

void tos_main() {
    tos::launch(tos::alloc_stack, mpu_task);
}