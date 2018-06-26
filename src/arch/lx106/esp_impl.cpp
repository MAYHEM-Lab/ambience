//
// Created by fatih on 4/26/18.
//

extern "C"
{
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include <mem.h>
#include <xtensa/config/core-isa.h>
#include <user_interface.h>
}

#include <tos/ft.hpp>

extern "C"
{
void tos_power_down()
{
}

static_assert(sizeof(int) == 4, "");
alignas(16) char stack[1024 * 2];
static int stack_index = 0;
void* tos_stack_alloc(size_t size)
{
    return stack + 1024 * stack_index++;
}

void tos_stack_free(void* data)
{
    //delete[] (char*)data;
}

#define __STRINGIFY(x) #x
#define xt_disable_interrupts(state, level) __asm__ __volatile__("rsil %0," __STRINGIFY(level) "; esync; isync; dsync" : "=a" (state))
#define xt_enable_interrupts(state)  __asm__ __volatile__("wsr %0,ps; esync" :: "a" (state) : "memory")

static uint32_t interruptsState;
void tos_enable_interrupts()
{
    xt_enable_interrupts(interruptsState);
}

void tos_disable_interrupts()
{
    xt_disable_interrupts(interruptsState, 15);
}
}

extern "C" void __cxa_pure_virtual()
{
}

void tos_init();
static void entry();

extern "C"
{
void ICACHE_FLASH_ATTR user_init()
{
    entry();
}
}

static void main_task(ETSEvent*)
{
    auto res = tos::kern::schedule();

    if (res == tos::exit_reason::yield)
    {
        system_os_post(tos::esp82::main_task_prio, 0, 0);
    }
}

static os_event_t arr[16];
static void entry()
{
    tos_init();

    system_os_task(main_task, tos::esp82::main_task_prio, arr, 16);
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}