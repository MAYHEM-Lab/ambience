//
// Created by fatih on 10/25/18.
//

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/compiler.hpp>
#include <tos/scheduler.hpp>
#include <tos/ft.hpp>
#include <tos_arch.hpp>
#include <libopencmsis/core_cm3.h>

extern "C"
{
extern "C" void *__dso_handle;
void *__dso_handle = 0;

void* tos_stack_alloc(size_t sz)
{
    return malloc(sz);
}

void tos_stack_free(void* ptr)
{
    return free(ptr);
}
}

void tos_main();

int main()
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz();

    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        //if (res == tos::exit_reason::restart) NVIC_SystemReset();// reboot();
        if (res == tos::exit_reason::power_down) __WFI();// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle) __asm__("wfe");// power_down(SLEEP_MODE_IDLE);
    }
}