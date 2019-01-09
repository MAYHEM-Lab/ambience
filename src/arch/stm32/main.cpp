//
// Created by fatih on 10/25/18.
//

#include <libopencm3/stm32/rcc.h>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/ft.hpp>
#include <tos_arch.hpp>

void tos_main();

int main()
{
#ifdef STM32F1
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
#else
    rcc_clock_setup_hse_3v3(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_120MHZ]);
#endif

    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) {
            tos_force_reset();
        }
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __WFI();// power_down(SLEEP_MODE_IDLE);
    }
}