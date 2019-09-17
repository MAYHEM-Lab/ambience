//
// Created by fatih on 11/30/18.
//

#include <arch/timer.hpp>

using tos::stm32::general_timer;

extern "C" void TIM2_IRQHandler()
{
    auto timer = general_timer::get(0);
    HAL_TIM_IRQHandler(timer->native_handle());
}

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *) {
    auto timer = general_timer::get(0);
    timer->run_callback();
}
