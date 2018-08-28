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
#include <tos/interrupt.hpp>
#include <lwip/timers.h>
#include <tos/compiler.hpp>
#include <tos/debug.hpp>

extern "C"
{
static_assert(sizeof(int) == 4, "");
alignas(16) char stack[4096 * 2];
static int stack_index = 0;
void* ICACHE_FLASH_ATTR tos_stack_alloc(size_t size)
{
    tos_debug_print("stack index: %d", stack_index);
    if (stack_index > 1)
    {
        while (true);
    }
    return stack + 4096 * stack_index++;
}

void ICACHE_FLASH_ATTR
tos_stack_free(void* data) {}

void ICACHE_FLASH_ATTR
tos_enable_interrupts()
{
    ets_intr_unlock();
}

void ICACHE_FLASH_ATTR
tos_disable_interrupts()
{
    ets_intr_lock();
}
}

void tos_main();
static void entry();

extern "C"
{
void ICACHE_FLASH_ATTR
user_init()
{
    system_init_done_cb(entry);
    //entry();
}
void ICACHE_FLASH_ATTR
user_rf_pre_init()
{
}
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
}

static void ICACHE_FLASH_ATTR
main_task(ETSEvent*)
{
    sys_check_timeouts();
    auto res = tos::kern::schedule();

    if (res == tos::exit_reason::yield)
    {
        system_os_post(tos::esp82::main_task_prio, 0, 0);
    }
}

extern "C"
{
extern void (*__init_array_start)();
extern void (*__init_array_end)();

static void ICACHE_FLASH_ATTR
do_global_ctors()
{
    void (**p)();
    for (p = &__init_array_start; p != &__init_array_end; ++p)
        (*p)();
}

uint32 ICACHE_FLASH_ATTR espconn_init(uint32)
{
    return 1;
}
}

static os_event_t arr[16];
static void ICACHE_FLASH_ATTR
entry()
{
    do_global_ctors();
    tos::kern::enable_interrupts();
    tos_main();
    system_set_os_print(1);
    system_os_task(main_task, tos::esp82::main_task_prio, arr, 16);
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}