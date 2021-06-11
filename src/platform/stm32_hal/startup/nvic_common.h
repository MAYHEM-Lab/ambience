#pragma once

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

struct [[gnu::packed]] nvic_common {
    using func_ptr = void (*)();
    void* stack_ptr;
    func_ptr reset;
    func_ptr nmi;
    func_ptr hard_fault;
    func_ptr mem_manage;
    func_ptr bus_fault;
    func_ptr usage_fault;
    func_ptr reserved_[4];
    func_ptr svc;
    func_ptr debug_mon;
    func_ptr reserved__;
    func_ptr pendsv;
    func_ptr sys_tick;

    static constexpr nvic_common default_handlers() {
        return {.stack_ptr = &_estack,
                .reset = Reset_Handler,
                .nmi = NMI_Handler,
                .hard_fault = HardFault_Handler,
                .mem_manage = MemManage_Handler,
                .bus_fault = BusFault_Handler,
                .usage_fault = UsageFault_Handler,
                .svc = SVC_Handler,
                .debug_mon = DebugMon_Handler,
                .pendsv = PendSV_Handler,
                .sys_tick = SysTick_Handler};
    }
};
