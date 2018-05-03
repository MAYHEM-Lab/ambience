//
// Created by fatih on 3/20/18.
//

#include <string.h>
#include <stdlib.h>
#include <iostream>

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

    void tos_shutdown()
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