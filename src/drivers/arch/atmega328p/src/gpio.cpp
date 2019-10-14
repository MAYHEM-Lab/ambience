//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <avr/interrupt.h>
#include <arch/gpio.hpp>
#include <avr/sleep.h>
#include <arch/exti.hpp>

ISR(INT0_vect)
{
    auto exti = tos::avr::exti::get(0);
    exti->isr(0);
}

ISR(INT1_vect)
{
    auto exti = tos::avr::exti::get(0);
    exti->isr(1);
}
