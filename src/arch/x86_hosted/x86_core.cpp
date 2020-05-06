//
// Created by fatih on 1/9/19.
//

#include <cstddef>
#include <cstdlib>
#include <csignal>

extern "C"
{
void tos_force_reset()
{
    raise(SIGTRAP);
    exit(1);
}

void tos_enable_interrupts()
{
}

void tos_disable_interrupts()
{
}
}
