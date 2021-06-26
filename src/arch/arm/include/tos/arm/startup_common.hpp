#pragma once

#include <tos/arm/nvic.hpp>

namespace tos::arm {
template<size_t IRQSize>
struct [[gnu::packed]] nvic_vector {
    using func_ptr = void (*)();
    tos::arm::vector_table common = tos::arm::vector_table::default_table();
    func_ptr ptrs[IRQSize];
};
}

extern "C" {
extern unsigned char _estack;

void Reset_Handler();
void NMI_Handler();
void HardFault_Handler();
void MemManage_Handler();
void BusFault_Handler();
void UsageFault_Handler();
void SVC_Handler();
void DebugMon_Handler();
void PendSV_Handler();
void SysTick_Handler();
}

namespace tos::arm {
constexpr vector_table vector_table::default_table() {
    return vector_table{.stack_ptr = &_estack,
        .reset = Reset_Handler,
        .nmi = NMI_Handler,
        .hard_fault = HardFault_Handler,
        .memory_fault = MemManage_Handler,
        .bus_fault = BusFault_Handler,
        .usage_fault = UsageFault_Handler,
        .svcall = SVC_Handler,
        .debug = DebugMon_Handler,
        .pendsv = PendSV_Handler,
        .systick = SysTick_Handler};
}
} // namespace tos::arm