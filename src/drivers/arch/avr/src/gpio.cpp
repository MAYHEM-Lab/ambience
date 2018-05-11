//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <avr/interrupt.h>
#include <gpio.hpp>

namespace tos
{
    namespace avr
    {
        tos::function_ref<void()> exint_handlers[2];
    }
}

ISR(INT0_vect)
{
    tos::avr::exint_handlers[0]();
}

ISR(INT1_vect)
{
    tos::avr::exint_handlers[1]();
}
