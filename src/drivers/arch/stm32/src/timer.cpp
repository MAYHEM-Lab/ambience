//
// Created by fatih on 11/30/18.
//

#include <timer.hpp>

/*void tim2_isr()
{
    if (timer_get_flag(TIM2, TIM_SR_CC1IF))
    {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);
        uint16_t compare_time = timer_get_counter(TIM2);
        timer_set_oc_value(TIM2, TIM_OC1, compare_time + 2);

    }
}*/