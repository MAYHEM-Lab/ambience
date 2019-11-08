//
// Created by fatih on 10/31/19.
//


#include <tos/ft.hpp>

enum IRQn_Type {};
#define __NVIC_PRIO_BITS 0
#include <core_cm4.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/dpl/HwiP.h>

extern "C" {
#include <NoRTOS.h>
#include <ti/drivers/Power.h>
}

extern void tos_main();

extern "C" {
__attribute__ ((section(".dbghdr"), used))
unsigned long ulDebugHeader[] = {
        0x5AA5A55A,
        0x000FF800,
        0xEFA3247D
};
}

int main() {
    NoRTOS_Config cfg;
    // Get current values of all configuration settings
    NoRTOS_getConfig(&cfg);
    // Change config settings we want to change while leaving other
    // settings at their default values ...
    // Change system "tick" frequency to 10,000 Hz
    cfg.clockTickPeriod = 100;
    // Change interrupt used for Swi scheduling to 11 (SVCall)
    cfg.swiIntNum = 11;
    cfg.idleCallback = Power_idleFunc;

    NoRTOS_setConfig(&cfg);
    // Start NoRTOS

    Board_init();
    NoRTOS_start();

    CoreDebug->DHCSR |= CoreDebug_DHCSR_C_DEBUGEN_Msk;
    //HwiP_enable();

    tos_main();

    while (true) {
        auto res = tos::sched.schedule();
        if (res == tos::exit_reason::restart) {
            tos_force_reset();
        }
        if (res == tos::exit_reason::power_down) {
            asm volatile("WFI");
        }
        if (res == tos::exit_reason::idle) {
            asm volatile("WFI");
        }
        if (res == tos::exit_reason::yield) {
            // Do nothing
        }
    }
}
