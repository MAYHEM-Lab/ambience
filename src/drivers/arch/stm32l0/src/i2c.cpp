//
// Created by fatih on 8/11/19.
//

#include <arch/i2c.hpp>

void i2c1_ev_isr() {
    const auto instance = tos::stm32::i2c::get(0);
    instance->isr();
}

void i2c1_er_isr() {
    const auto instance = tos::stm32::i2c::get(0);
    instance->err_isr();
}

void i2c2_ev_isr() {
    const auto instance = tos::stm32::i2c::get(1);
    instance->isr();
}

void i2c2_er_isr() {
    const auto instance = tos::stm32::i2c::get(1);
    instance->err_isr();
}