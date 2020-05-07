//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "tos_platform.hpp"
#include <tos/compiler.hpp>

extern "C"
{
    inline void __attribute__((always_inline)) tos_set_stack_ptr(char* ptr)
    {
        // return address is in the stack
        // if we just change the stack pointer
        // we can't return to the caller
        // copying the last 10 bytes from the original
        // stack to this stack so that we'll be able
        // to return
        //  memcpy(ptr - 2, (void*)SP, 2);
        SP = reinterpret_cast<uint16_t>(ptr);
    }

    inline void* __attribute__((always_inline)) tos_get_stack_ptr()
    {
        return reinterpret_cast<void*>(SP);
    }

    inline void tos_enable_interrupts() __attribute__((always_inline));
    inline void tos_enable_interrupts()
    {
        sei();
    }

    inline void tos_disable_interrupts() __attribute__((always_inline));
    inline void tos_disable_interrupts()
    {
        cli();
    }
}