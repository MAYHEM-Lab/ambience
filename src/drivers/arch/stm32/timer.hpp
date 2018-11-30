//
// Created by fatih on 11/30/18.
//

#pragma once

#include <cstdint>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencmsis/core_cm3.h>

namespace tos::stm32
{
    template <bool IsGen> class timer_base;

    struct tim_def
    {
        uint32_t tim;
        rcc_periph_clken rcc;
        rcc_periph_rst rst;
        uint8_t irq;
    };

    namespace timers
    {
        constexpr tim_def tim2 {
            TIM2, RCC_TIM2, RST_TIM2, NVIC_TIM2_IRQ
        };
        constexpr tim_def tim3 {
            TIM3, RCC_TIM3, RST_TIM3, NVIC_TIM3_IRQ
        };
    } // namespace timers

    using gp_timers = timer_base<true>;

    template <>
    class timer_base<true>
    {
    public:
        timer_base(const tim_def& tim);

        void set_frequency(uint16_t hertz);

        void enable();
        void disable();
    private:
        const tim_def* m_def;
    };

    constexpr bool is_general_timer(int num)
    {
        return num >= 2 && num <= 5;
    }

    template <int TmrNum>
    using timer = timer_base<is_general_timer(TmrNum)>;

} // namespace tos::stm32

// impl

namespace tos::stm32
{
    timer_base<true>::timer_base(const tim_def& def) : m_def{&def}
    {
        rcc_periph_clock_enable(m_def->rcc);
        rcc_periph_reset_pulse(m_def->rst);
        timer_set_mode(m_def->tim, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
        timer_disable_preload(m_def->tim);
        timer_continuous_mode(m_def->tim);
        timer_set_period(m_def->tim, 65535);
    }

    void timer_base<true>::set_frequency(uint16_t hertz) {
        timer_set_prescaler(m_def->tim, ((rcc_apb1_frequency * 2) / 2'000));
        timer_set_oc_value(m_def->tim, TIM_OC1, ((uint32_t)hertz * 2) / 1000);
    }

    void timer_base<true>::enable() {
        timer_enable_counter(m_def->tim);
        nvic_enable_irq(m_def->irq);
        timer_enable_irq(m_def->tim, TIM_DIER_CC1IE);
    }

    void timer_base<true>::disable() {
        timer_disable_irq(m_def->tim, TIM_DIER_CC1IE);
        nvic_disable_irq(m_def->irq);
        timer_disable_counter(m_def->tim);
    }
} // namespace tos::stm32