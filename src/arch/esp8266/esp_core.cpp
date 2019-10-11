//
// Created by fatih on 1/9/19.
//

extern "C" {
#include "ets_sys.h"
#include "gpio.h"
#include "os_type.h"
#include "osapi.h"
#include <mem.h>
#include <user_interface.h>
#include <xtensa/config/core-isa.h>
}

#include <tos/compiler.hpp>

extern "C" {
static_assert(sizeof(int) == 4, "");

void* ICACHE_FLASH_ATTR tos_stack_alloc(size_t size) {
    auto res = os_malloc(size);
    if (res == nullptr) {
        while (true)
            ;
    }
    return res;
}

void ICACHE_FLASH_ATTR tos_stack_free(void* data) { os_free(data); }

void ICACHE_FLASH_ATTR tos_enable_interrupts() { ets_intr_unlock(); }

void ICACHE_FLASH_ATTR tos_disable_interrupts() { ets_intr_lock(); }

[[noreturn ]]void tos_force_reset() {
    // esp sdk should reset
    while (true) {}
}
}
