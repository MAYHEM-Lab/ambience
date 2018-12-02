//
// Created by fatih on 11/30/18.
//

#include <timer.hpp>
#include <memory>
#include <tos/track_ptr.hpp>

void tim2_isr()
{
    if (timer_get_flag(TIM2, TIM_SR_CC1IF))
    {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);
        uint16_t compare_time = timer_get_counter(TIM2);

        using namespace tos::stm32;
        timer_set_oc_value(TIM2, TIM_OC1, compare_time + get_period(*timer_base<true>::get<0>()));
        run_callback(*timer_base<true>::get<0>());
    }
}
