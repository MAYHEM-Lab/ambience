//
// Created by fatih on 5/25/19.
//

#include <arch/spi.hpp>

void spi1_isr()
{
    const auto instance = tos::stm32::spi::get(0);
    instance->isr();
}

void spi2_isr()
{
    const auto instance = tos::stm32::spi::get(1);
    instance->isr();
}