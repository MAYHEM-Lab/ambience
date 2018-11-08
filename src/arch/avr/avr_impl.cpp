//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#include <avr/io.h>
#include <avr/wdt.h>

#include <tos_arch.hpp>
#include <tos/arch.hpp>

#include <avr/power.h>
#include <avr/sleep.h>
#include <tos/compiler.hpp>
#include <tos/ft.hpp>
#include <ft/include/tos/semaphore.hpp>
#include <drivers/arch/avr/gpio.hpp>

extern "C"
{
    /**
     * Doing a software reset through watchdog timer leaves the wdt
     * enabled after the reset which causes an infinite reset loop.
     *
     * Disabling the watchdog timer at reset solves it.
     *
     * .init3 is simply an unused section that's reserved for user code,
     * so we stick it there.
     */
    void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
    void wdt_init(void)
    {
        MCUSR = 0;
        wdt_disable();
    }

    alignas(alignof(std::max_align_t)) char stack[256*2];
    int stack_index = 0;
    void* tos_stack_alloc(size_t)
    {
        return stack+256*stack_index++;
    }

    void tos_stack_free(void*)
    {
    }
}

extern void tos_main();

/*static void reboot()
{
    wdt_enable(WDTO_15MS);
    for (;;);
}*/

static void power_down(int mode)
{
    set_sleep_mode(mode);
    sleep_enable();
    sleep_bod_disable();
    sleep_cpu();
    sleep_disable();
}

tos::event s;
ISR (WDT_vect)
{
    s.fire_isr();
    // WDIE & WDIF is cleared in hardware upon entering this ISR
    wdt_disable();
}

void wait_wdt()
{
    s.wait();
}

int TOS_EXPORT TOS_MAIN NORETURN main()
{
    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart) {
            tos_main();
        }
        if (res == tos::exit_reason::power_down) {
            power_down(SLEEP_MODE_PWR_DOWN);
        }
        if (res == tos::exit_reason::idle) power_down(SLEEP_MODE_IDLE);
    }

    __builtin_unreachable();
}
