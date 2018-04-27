//
// Created by fatih on 4/26/18.
//

#include <tos.hpp>

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

extern "C"
{
void tos_power_down()
{
}

static_assert(sizeof(int) == 4, "");
alignas(16) char stack[1024 * 2];
int stack_index = 0;
void* tos_stack_alloc(size_t size)
{
    return stack + 1024 * stack_index++;
}

void tos_stack_free(void* data)
{
    //delete[] (char*)data;
}

void tos_shutdown()
{
}

#define __STRINGIFY(x) #x
#define xt_disable_interrupts(state, level) __asm__ __volatile__("rsil %0," __STRINGIFY(level) "; esync; isync; dsync" : "=a" (state))
#define xt_enable_interrupts(state)  __asm__ __volatile__("wsr %0,ps; esync" :: "a" (state) : "memory")

uint32_t interruptsState;
void tos_enable_interrupts()
{
    xt_enable_interrupts(interruptsState);
}

void tos_disable_interrupts()
{
    xt_disable_interrupts(interruptsState, 15);
}
}

void* operator new(size_t sz)
{
    return os_malloc(sz);
}

void* operator new[](size_t sz)
{
    return os_malloc(sz);
}

extern "C" void __cxa_pure_virtual()
{
}

void operator delete[](void* ptr)
{
    os_free(ptr);
}

void operator delete(void* ptr)
{
    os_free(ptr);
}

void operator delete(void* ptr, size_t)
{
    os_free(ptr);
}

void operator delete[](void* ptr, size_t)
{
    os_free(ptr);
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

static volatile os_timer_t some_timer;

static void some_timerfunc(void *arg)
{
    system_os_post(USER_TASK_PRIO_1, 0, 0);
}

static void main_task(ETSEvent*)
{
    tos::schedule();

}

static os_event_t arr[16];
static void entry()
{
    tos_init();

    system_os_task(main_task, USER_TASK_PRIO_1, arr, 16);
    system_os_post(USER_TASK_PRIO_1, 0, 0);

    os_timer_setfn(const_cast<os_timer_t*>(&some_timer), (os_timer_func_t *)some_timerfunc, NULL);
    os_timer_arm(const_cast<os_timer_t*>(&some_timer), 500, 1);
}