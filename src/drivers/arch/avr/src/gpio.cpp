//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <avr/interrupt.h>
#include <arch/avr/gpio.hpp>
#include <avr/sleep.h>

namespace tos
{
    namespace avr
    {
        tos::function_ref<void()> exint_handlers[2] = {
            { +[](void*){}, nullptr },
            { +[](void*){}, nullptr }
        };
    }
}

ISR(INT0_vect)
{
    sleep_disable();

    tos::avr::exint_handlers[0]();
}

ISR(INT1_vect)
{
    tos::avr::exint_handlers[1]();
}
