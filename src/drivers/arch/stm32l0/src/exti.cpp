//
// Created by fatih on 7/24/19.
//

#include <arch/exti.hpp>
#include <libopencm3/stm32/exti.h>

extern "C" {
    void exti0_isr()
    {

    }
    void exti1_isr()
    {

    }
    void exti2_isr()
    {

    }
    void exti3_isr()
    {

    }
    void exti4_isr()
    {

    }
    void exti9_5_isr()
    {
        if ((EXTI_PR & EXTI6) == EXTI6)
        {
            EXTI_PR = EXTI6;
            auto it = tos::stm32::exti::get(0);
            if (!it)
            {
                tos::kern::fatal("No EXTI driver instance!");
            }
            it->isr(EXTI6);
        }
    }
    void exti15_10_isr()
    {

    }
}