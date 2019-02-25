//
// Created by fatih on 2/22/19.
//

#include <tos_arch.hpp>
#include <tos/compiler.hpp>
#include <cstdlib>

extern "C"
{
    void NORETURN tos_force_reset()
    {
        NVIC_SystemReset();
    }

alignas(8) char stack[512*2];
int stack_index = 0;
void* tos_stack_alloc(size_t sz)
{
    return malloc(sz);
    return stack+512*stack_index++;
}

void tos_stack_free(void* ptr)
{
    return free(ptr);
}
}

