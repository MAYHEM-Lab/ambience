//
// Created by fatih on 11/30/18.
//

#include <arch/timer.hpp>
#include <memory>
#include <tos/track_ptr.hpp>

void tim2_isr()
{
    tos::stm32::general_timer::get(0)->isr();
}

void tim3_isr()
{
    tos::stm32::general_timer::get(1)->isr();
}
