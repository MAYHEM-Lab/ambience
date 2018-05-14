//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#include <tos_arch.hpp>
#include <tos/arch.hpp>

#include <stdlib.h>

#include <string.h>

#include <avr/power.h>
#include <avr/sleep.h>
#include <util/delay.h>

extern "C"
{
#define soft_reset()        \
    do                          \
    {                           \
        wdt_enable(WDTO_15MS);  \
        for(;;)                 \
        {                       \
        }                       \
    } while(0)
// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));

// Function Implementation
void wdt_init(void)
{
    MCUSR = 0;
    wdt_disable();

    return;
}


void tos_power_down()
{
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    //sleep_bod_disable();
    sleep_cpu();
    sleep_disable();
}

alignas(16) char stack[256 * 2];
int stack_index = 0;
void* tos_stack_alloc(size_t size)
{
    return stack + 256 * stack_index++;
}

void tos_stack_free(void* data)
{
    //delete[] (char*)data;
}

void tos_shutdown()
{
    soft_reset();
}
}

void* operator new(size_t sz)
{
    return malloc(sz);
}

void* operator new[](size_t sz)
{
    return malloc(sz);
}

extern "C" void __cxa_pure_virtual()
{
}

void operator delete[](void* ptr)
{
    free(ptr);
}

void operator delete(void* ptr)
{
    free(ptr);
}

void operator delete(void* ptr, size_t)
{
    free(ptr);
}

void operator delete[](void* ptr, size_t)
{
    free(ptr);
}
