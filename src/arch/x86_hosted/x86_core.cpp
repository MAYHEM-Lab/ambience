//
// Created by fatih on 1/9/19.
//

#include <cstddef>
#include <cstdlib>

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

void tos_force_reset()
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
