#pragma once

#include "gpio.hpp"

#include <common/gpio.hpp>
#include <etl/flat_map.h>
#include <stm32_hal/ll_exti.hpp>
#include <tos/function_ref.hpp>
#include <tos/interrupt.hpp>

namespace tos {
namespace stm32 {
class l0_exti : public tracked_driver<l0_exti> {
public:
    l0_exti()
        : tracked_driver(0) {
#if defined(STM32L0)
        HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
        HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
        HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
        HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
        HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
#elif defined(STM32L4)
        HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
#endif
    }

    using pin_type = gpio::pin_type;

    void attach(pin_type pin, pin_change::rising_t, function_ref<void()> fun) {
        tos::int_guard ig;
        m_handlers.emplace(pin.pin, fun);

        GPIO_InitTypeDef gpio{};

        enable_rcc(pin.port);
        gpio.Pin = pin.pin;
        gpio.Mode = GPIO_MODE_IT_RISING;
        gpio.Pull = GPIO_PULLDOWN;
        gpio.Speed = GPIO_SPEED_LOW;
        
        HAL_GPIO_Init(pin.port, &gpio);
    }

    void attach(pin_type pin, pin_change::falling_t, function_ref<void()> fun) {
        tos::int_guard ig;
        m_handlers.emplace(pin.pin, fun);

        GPIO_InitTypeDef gpio{};

        enable_rcc(pin.port);
        gpio.Pin = pin.pin;
        gpio.Mode = GPIO_MODE_IT_FALLING;
        gpio.Pull = GPIO_PULLDOWN;
        gpio.Speed = GPIO_SPEED_LOW;

        HAL_GPIO_Init(pin.port, &gpio);
    }

    void detach(pin_type pin) {
        auto it = m_handlers.find(pin.pin);

        HAL_GPIO_DeInit(pin.port, pin.pin);

        m_handlers.erase(it);
    }

    void isr(uint16_t pin) {
        auto it = m_handlers.find(pin);
        if (it == m_handlers.end()) {
            tos::debug::panic("EXTI Handler not found!");
        }
        it->second();
    }

private:
    etl::flat_map<uint16_t, function_ref<void()>, 16> m_handlers;
};

using exti = l0_exti;
} // namespace stm32
} // namespace tos
