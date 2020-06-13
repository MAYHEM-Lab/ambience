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
[[noreturn]] void tos_force_reset() {
    // esp sdk should reset
    while (true) {
    }
}
}
