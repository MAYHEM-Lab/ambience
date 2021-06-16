//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <app_util_platform.h>
#include <nrf.h>
#include <nrf_pwr_mgmt.h>
#include <tos/arm/exception.hpp>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

#ifdef SOFTDEVICE_PRESENT
#include "nrf_sdh.h"
#include "nrf_soc.h"
#endif // SOFTDEVICE_PRESENT

void tos_main();

extern "C" void HardFault_Handler() {
    tos::arm::exception::hard_fault();
}

extern "C" void UsageFault_Handler() {
    tos::arm::exception::usage_fault();
}

extern "C" void MemoryManagement_Handler() {
    tos::arm::exception::mem_fault();
}

extern "C" void BusFault_Handler() {
    tos::arm::exception::hard_fault();
}

extern "C" [[gnu::weak]] void SVC_Handler() {
    tos::arm::exception::out_svc_handler();
}

extern "C" {
int main();
extern void (*start_ctors[])();
extern void (*end_ctors[])();
void _start() {
    main();
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

    tos_main();

    while (true) {
        {
            tos::int_guard ig;
            auto res = tos::global::sched.schedule(ig);
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
}