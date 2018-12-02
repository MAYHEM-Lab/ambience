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

        auto ptr = timer_base<true>::get(0);
        if (!ptr)
        {
            return;
        }

        timer_set_oc_value(TIM2, TIM_OC1, compare_time + get_period(*ptr));
        run_callback(*ptr);
    }
}

void tim3_isr()
{
    if (timer_get_flag(TIM3, TIM_SR_CC1IF))
    {
        timer_clear_flag(TIM3, TIM_SR_CC1IF);
        uint16_t compare_time = timer_get_counter(TIM3);

        using namespace tos::stm32;

        auto ptr = timer_base<true>::get(1);
        if (!ptr)
        {
            return;
        }

        timer_set_oc_value(TIM3, TIM_OC1, compare_time + get_period(*ptr));
        run_callback(*ptr);
    }
}
