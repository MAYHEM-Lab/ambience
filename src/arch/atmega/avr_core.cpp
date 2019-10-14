//
// Created by fatih on 1/9/19.
//

#include <avr/delay.h>

#include <avr/io.h>
#include <avr/wdt.h>

#include <tos/compiler.hpp>
#include <tos_arch.hpp>
#include <cstddef>

[[noreturn]] void reset_cpu()
{
    tos_disable_interrupts();
    wdt_enable(WDTO_15MS);
    while (true){}
}

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
void __attribute__((naked, used, section(".init3"))) wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();
}

[[noreturn]] void tos_force_reset()
{
    reset_cpu();
}

alignas(alignof(std::max_align_t)) static char stack[512];
int stack_index = 0;
void* tos_stack_alloc(size_t sz)
{
    return stack + sz * stack_index++;
}

void tos_stack_free(void*)
{
}
}
