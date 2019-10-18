//
// Created by fatih on 11/30/18.
//

#include <arch/timer.hpp>

using tos::stm32::general_timer;

namespace {
int get_tim_num(TIM_HandleTypeDef* handle)
{
    switch (uintptr_t(handle->Instance))
    {
        case TIM2_BASE: return 2;
#if defined(TIM3_BASE)
        case TIM3_BASE: return 3;
#endif
    }
    return 0;
}
}
extern "C" {
void TIM2_IRQHandler() {
    auto timer = general_timer::get(0);
    HAL_TIM_IRQHandler(timer->native_handle());
}

void TIM3_IRQHandler() {
    auto timer = general_timer::get(1);
    HAL_TIM_IRQHandler(timer->native_handle());
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * handle) {
    auto timer = general_timer::get(get_tim_num(handle) - 2);
    timer->run_callback();
}
}