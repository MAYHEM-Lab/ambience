//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <app_util_platform.h>
#include <nrf.h>
#include <nrf52840.h>
#include <nrf_pwr_mgmt.h>
#include <system_nrf52840.h>
#include <tos/arm/exception.hpp>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_soc.h"
#endif // SOFTDEVICE_PRESENT

#include <tos/debug/trace/metrics/counter.hpp>

tos::trace::basic_counter systicks;

void tos_main();
extern "C" void SysTick_Handler() {
    systicks.inc();
}

extern "C" void HardFault_Handler() {
    tos::arm::exception::hard_fault();
}

extern "C" void UsageFault_Handler() {
    tos::arm::exception::usage_fault();
}

extern "C" void MemManage_Handler() {
    tos::arm::exception::mem_fault();
}

extern "C" void BusFault_Handler() {
    tos::arm::exception::hard_fault();
}

extern "C" [[gnu::weak]] void SVC_Handler() {
    tos::arm::exception::out_svc_handler();
}

extern "C" {
void abort() {
    tos::debug::panic("abort called");
}
void __cxa_atexit() {}
}

extern "C" {
int main();
extern void (*start_ctors[])();
extern void (*end_ctors[])();
void _start() {
    main();
    TOS_UNREACHABLE();
}
}
extern "C" int main() {
    std::for_each(start_ctors, end_ctors, [](auto ctor) { ctor(); });

    tos::kern::enable_interrupts();

    nrf_pwr_mgmt_init();

    NVIC_EnableIRQ(UsageFault_IRQn);
    NVIC_SetPriority(UsageFault_IRQn, 0);
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;

    NVIC_EnableIRQ(BusFault_IRQn);
    NVIC_SetPriority(BusFault_IRQn, 0);
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;

    SCB->SCR |= SCB_SCR_SEVONPEND_Msk;

    SysTick_Config(SystemCoreClock / (1000 * 50));
    NVIC_EnableIRQ(SysTick_IRQn);
    NVIC_SetPriority(SysTick_IRQn, 0);

    tos_main();

    while (true) {
        auto res = tos::global::sched.schedule(tos::int_guard{});
        if (res == tos::exit_reason::restart)
            NVIC_SystemReset(); // reboot();
        if (res == tos::exit_reason::power_down || res == tos::exit_reason::idle) {
            if (nrf_sdh_is_enabled()) {
                ret_code_t ret_code = sd_app_evt_wait();
                ASSERT((ret_code == NRF_SUCCESS) ||
                        (ret_code == NRF_ERROR_SOFTDEVICE_NOT_ENABLED));
                UNUSED_VARIABLE(ret_code);
            } else {
                __WFE();
                __SEV();
                __WFE();
            }
        }
    }
}