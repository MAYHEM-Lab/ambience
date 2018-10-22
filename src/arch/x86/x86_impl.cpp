//
// Created by fatih on 3/20/18.
//

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>

extern "C"
{
    void tos_power_down()
    {
    }

    void* tos_stack_alloc(size_t size)
    {
        return malloc(size);
    }

    void tos_stack_free(void* data)
    {
        free(data);
    }

    void tos_reboot()
    {
        exit(1);
    }

    void tos_enable_interrupts()
    {

    }

    void tos_disable_interrupts()
    {

    }
}

void tos_main();

extern "C" int main()
{
    tos::kern::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::kern::schedule();
        if (res == tos::exit_reason::restart)
        {
            return 0;
        }
    }
}