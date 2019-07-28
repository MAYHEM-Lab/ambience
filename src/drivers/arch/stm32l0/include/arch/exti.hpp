//
// Created by fatih on 7/22/19.
//

#pragma once

#include "gpio.hpp"
#include <common/gpio.hpp>
#include <tos/function_ref.hpp>
#include <etl/flat_map.h>
#include <libopencm3/stm32/exti.h>
#include <tos/interrupt.hpp>

namespace tos
{
namespace stm32
{
    class exti : public tracked_driver<exti>
    {
    public:
        exti() : tracked_driver(0) {}

        using pin_type = gpio::pin_type;

        void attach(pin_type pin, pin_change::rising_t, function_ref<void()> fun)
        {
            tos::int_guard ig;
            const auto exti = pin.pin; // already in the form of 1 << PIN_ID
            m_handlers.emplace(pin.pin, fun);

            exti_select_source(exti, pin.port->which);
            exti_set_trigger(exti, EXTI_TRIGGER_RISING);
            exti_enable_request(exti);

#if defined(STM32L4)
            nvic_enable_irq(NVIC_EXTI9_5_IRQ);
#elif defined(STM32L0)
            nvic_enable_irq(NVIC_EXTI0_1_IRQ);
            nvic_enable_irq(NVIC_EXTI2_3_IRQ);
            nvic_enable_irq(NVIC_EXTI4_15_IRQ);
#endif
        }

        void detach(pin_type pin)
        {
            auto it = m_handlers.find(pin.pin);
            const auto exti = it->first;

            exti_disable_request(exti);

            m_handlers.erase(it);
        }

        void isr(uint16_t pin)
        {
            auto it = m_handlers.find(pin);
            if (it == m_handlers.end())
            {
                tos::kern::fatal("EXTI Handler not found!");
            }
            it->second();
        }

    private:

        etl::flat_map<uint16_t, function_ref<void()>, 16> m_handlers;
    };
} // namespace stm32
} // namespace tos
