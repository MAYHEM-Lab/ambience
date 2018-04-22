//
// Created by Mehmet Fatih BAKIR on 28/03/2018.
//

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/atomic.h>

#include "tos/atomic.hpp"
#include "tos/ft.hpp"
#include "tos/thread_info.hpp"
#include <tos/char_stream.hpp>

#include <tos_arch.hpp>
#include <tos/arch.hpp>

#include <stdlib.h>

#include <string.h>

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

void tos_set_stack_ptr(void* ptr) __attribute__((naked));
void tos_set_stack_ptr(void* ptr)
{
    // return address is in the stack
    // if we just change the stack pointer
    // we can't return to the caller
    // copying the last 10 bytes from the original
    // stack to this stack so that we'll be able
    // to return
    memcpy(ptr - 10, (void*)SP, 10);
    SP = reinterpret_cast<uint16_t>(ptr - 10);
}

void tos_power_down()
{
}

char stack[256 * 2];
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

void tos_enable_interrupts()
{
    sei();
}

void tos_disable_interrupts()
{
    cli();
}
}

namespace tos
{
    template <class T>
    void atomic<T>::add(const T& t)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_t += t;
        }
    }

    template class atomic<int8_t>;
}

char buf[sizeof(tos::thread_info) * 2];
int index = 0;

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
