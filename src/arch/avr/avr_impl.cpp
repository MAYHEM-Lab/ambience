//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#include <avr/io.h>
#include <avr/wdt.h>

#include <tos_arch.hpp>
#include <tos/arch.hpp>

#include <avr/power.h>
#include <avr/sleep.h>

extern "C"
{
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
}

void tos_power_down()
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
}

alignas(16) char stack[256*2];
int stack_index = 0;
void* tos_stack_alloc(size_t size)
{
    return stack+256*stack_index++;
}

void tos_stack_free(void* data)
{
}

void tos_reboot()
{
    wdt_enable(WDTO_15MS);
    for (;;);
}

void __cxa_pure_virtual()
{
}
}

