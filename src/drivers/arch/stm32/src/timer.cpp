//
// Created by fatih on 11/30/18.
//

#include <timer.hpp>
#include <memory>

namespace tos::stm32
{
    std::weak_ptr<tos::stm32::gp_timers> tmr2;
}

void tim2_isr()
{
    if (timer_get_flag(TIM2, TIM_SR_CC1IF))
    {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);
        uint16_t compare_time = timer_get_counter(TIM2);

        auto x = tos::stm32::tmr2.lock();
        timer_set_oc_value(TIM2, TIM_OC1, compare_time + get_period(*x));
        run_callback(*x);
    }
}
