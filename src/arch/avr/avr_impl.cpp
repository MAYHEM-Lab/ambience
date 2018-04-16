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
}

namespace tos
{
    void init()
    {
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

class null_stream : public tos::char_ostream
{
public:
    int write(const char* buf, size_t sz) override
	{
		return sz;
	}
	void putc(char c) override
	{
	}
};

char buf[sizeof(ft::thread_info) * 2];
int index = 0;

void* operator new(size_t)
{
    return buf + index++ * sizeof(ft::thread_info);
}

void* operator new[](size_t)
{
    return buf + index++ * sizeof(ft::thread_info);
}

extern "C" void __cxa_pure_virtual()
{

}

void operator delete[](void*)
{

}

void operator delete(void*)
{

}

void operator delete(void*, size_t)
{

}

namespace tos
{
    namespace arch
    {
        char_ostream* debug_stream()
        {
            static null_stream str;
            return &str;
        }
    }
}
