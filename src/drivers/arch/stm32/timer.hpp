//
// Created by fatih on 11/30/18.
//

#pragma once

#include <cstdint>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>
#include <util/include/tos/function_ref.hpp>
#include <memory>
#include <tos/track_ptr.hpp>
#include <drivers/common/timer.hpp>
#include <drivers/common/driver_base.hpp>

namespace tos::stm32
{
    namespace detail
    {
        /**
         * This struct is a literal type that just collects
         * relevant data about a specific general purpose timer.
         *
         * DO NOT instantiate objects of this type!
         */
        struct gen_tim_def
        {
            uint32_t tim;
            rcc_periph_clken rcc;
            rcc_periph_rst rst;
            uint8_t irq;
        };

        constexpr detail::gen_tim_def gen_timers[] = {
                {TIM2, RCC_TIM2, RST_TIM2, NVIC_TIM2_IRQ},
                {TIM3, RCC_TIM3, RST_TIM3, NVIC_TIM3_IRQ}
        };
    } // namespace detail

    /**
     * STM32s have a bunch of timers. Some of these timers are advanced,
     * and some of them are called general purpose.
     *
     * This class implements the Tos timer interface using the 1st channel
     * of a given general purpose timer.
     */
    class general_timer :
            public self_pointing<general_timer>,
            public tracked_driver<general_timer, 3>
    {
    public:
        explicit general_timer(const detail::gen_tim_def& def);

        void set_frequency(uint16_t hertz);

        void set_callback(tos::function_ref<void()> fun) {
            m_fun = fun;
        }

        void enable();
        void disable();
    private:
        const detail::gen_tim_def* m_def;
        tos::function_ref<void()> m_fun;
        uint16_t m_period;
        friend void run_callback(general_timer& tmr){
            tmr.m_fun();
        }
        friend uint16_t get_period(general_timer& tmr)
        {
            return tmr.m_period;
        }
    };
} // namespace tos::stm32

namespace tos
{
    inline stm32::general_timer open_impl(tos::devs::timer_t<2>)
    {
        return stm32::general_timer{ stm32::detail::gen_timers[0] };
    }

    inline stm32::general_timer open_impl(tos::devs::timer_t<3>)
    {
        return stm32::general_timer{ stm32::detail::gen_timers[1] };
    }
} // namespace tos

// impl

namespace tos::stm32
{
    inline general_timer::general_timer(const detail::gen_tim_def& def)
        : tracked_driver(std::distance(detail::gen_timers, &def)), m_def{&def}, m_fun{[](void*){}}
    {
        rcc_periph_clock_enable(m_def->rcc);
        rcc_periph_reset_pulse(m_def->rst);
        timer_set_mode(m_def->tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        timer_disable_preload(m_def->tim);
        timer_continuous_mode(m_def->tim);
        timer_set_period(m_def->tim, 65535);
    }

    inline void general_timer::set_frequency(uint16_t hertz) {
        // for 1000 hertz, we need a period of 2, thus hertz * 2 / 1000
        m_period = ((hertz * 2) / 1'000);
        timer_set_prescaler(m_def->tim, ((rcc_apb1_frequency * 2) / 2'000));
    }

    inline void general_timer::enable() {
        timer_set_oc_value(m_def->tim, TIM_OC1, timer_get_counter(m_def->tim) + m_period);
        timer_enable_counter(m_def->tim);
        nvic_enable_irq(m_def->irq);
        timer_enable_irq(m_def->tim, TIM_DIER_CC1IE);
    }

    inline void general_timer::disable() {
        timer_disable_irq(m_def->tim, TIM_DIER_CC1IE);
        nvic_disable_irq(m_def->irq);
        timer_disable_counter(m_def->tim);
    }
} // namespace tos::stm32