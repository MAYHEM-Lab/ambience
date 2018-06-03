//
// Created by Mehmet Fatih BAKIR on 01/06/2018.
//

#include <stdbool.h>
#include <stdint.h>
#include <tos/ft.hpp>

extern "C"
{
alignas(8) char stack[512*2];
int stack_index = 0;
void* tos_stack_alloc(size_t)
{
    return stack+512*stack_index++;
}

void tos_stack_free(void*)
{
}
}

void tos_main();

int main()
{
   // tos::enable_interrupts();

    tos_main();

    while (true)
    {
        auto res = tos::schedule();
        if (res == tos::exit_reason::restart);// reboot();
        if (res == tos::exit_reason::power_down);// power_down(SLEEP_MODE_PWR_DOWN);
        if (res == tos::exit_reason::idle);// power_down(SLEEP_MODE_IDLE);
    }
}