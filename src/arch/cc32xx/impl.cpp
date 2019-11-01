//
// Created by fatih on 10/31/19.
//


#include <tos/ft.hpp>

extern "C" {
#include <NoRTOS.h>
#include <ti/drivers/Power.h>
}

extern void tos_main();

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
    NoRTOS_start();

    tos_main();

    while (true) {
        auto res = tos::kern::schedule();
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
