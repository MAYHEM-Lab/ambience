//
// Created by fatih on 6/8/18.
//

#include <timer.hpp>
#include <nrfx_timer.h>
#include <drivers/include/nrfx_timer.h>
#include <nrf52.h>
#include <tos/interrupt.hpp>

namespace tos
{
    namespace nrf52
    {
        static constexpr nrfx_timer_t tmr = { NRF_TIMER0, NRFX_TIMER0_INST_IDX, TIMER0_CC_NUM };

        timer0::timer0() : m_cb(+[](void*){}) {
            nrfx_timer_config_t conf;
            conf.frequency = NRF_TIMER_FREQ_16MHz;
            conf.interrupt_priority = 7;
            conf.bit_width = NRF_TIMER_BIT_WIDTH_32;
            conf.mode = NRF_TIMER_MODE_TIMER;
            conf.p_context = this;

            nrfx_timer_init(&tmr, &conf, [](nrf_timer_event_t, void * p_context){
                auto self = static_cast<timer0*>(p_context);
                self->m_cb();
                self->write_ticks(self->m_ticks);
            });

            disable();
        }

        void timer0::set_frequency(uint16_t hertz) {
            auto ticks = nrfx_timer_us_to_ticks(&tmr, 1'000'000 / hertz);
            m_ticks = ticks;
        }

        void timer0::write_ticks(uint32_t ticks) {
            nrfx_timer_compare(&tmr, NRF_TIMER_CC_CHANNEL0, ticks, true);
            nrfx_timer_clear(&tmr);
            tos::refresh_interrupts();
        }

        void timer0::enable() {
            nrfx_timer_enable(&tmr);
            write_ticks(m_ticks);
            tos::refresh_interrupts();
        }

        void timer0::disable() {
            nrfx_timer_disable(&tmr);
            tos::refresh_interrupts();
        }

        uint16_t timer0::get_ticks() {
            return nrfx_timer_capture(&tmr, NRF_TIMER_CC_CHANNEL0);
        }
    }
}