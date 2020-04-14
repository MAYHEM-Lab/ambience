//
// Created by fatih on 6/8/18.
//

#include <arch/timer.hpp>
#include <drivers/include/nrfx_timer.h>
#include <nrf.h>
#include <nrfx_timer.h>
#include <tos/interrupt.hpp>

namespace tos {
namespace nrf52 {
static const nrfx_timer_t tmrs[] = {{NRF_TIMER0, NRFX_TIMER0_INST_IDX, TIMER0_CC_NUM},
                                    {NRF_TIMER1, NRFX_TIMER1_INST_IDX, TIMER1_CC_NUM},
                                    {NRF_TIMER2, NRFX_TIMER2_INST_IDX, TIMER2_CC_NUM}};

timer0::timer0(int idx)
    : m_cb(+[](void*) {})
    , m_idx{idx} {
    nrfx_timer_config_t conf;
    conf.frequency = NRF_TIMER_FREQ_16MHz;
    conf.interrupt_priority = 7;
    conf.bit_width = NRF_TIMER_BIT_WIDTH_32;
    conf.mode = NRF_TIMER_MODE_TIMER;
    conf.p_context = this;

    nrfx_timer_init(&tmrs[m_idx], &conf, [](nrf_timer_event_t, void* p_context) {
        auto self = static_cast<timer0*>(p_context);
        self->m_cb();
        self->write_ticks(self->m_ticks);
    });

    disable();
}

void timer0::set_frequency(uint16_t hertz) {
    auto ticks = nrfx_timer_us_to_ticks(&tmrs[m_idx], 1'000'000 / hertz);
    m_ticks = ticks;
}

void timer0::write_ticks(uint32_t ticks) {
    nrfx_timer_compare(&tmrs[m_idx], NRF_TIMER_CC_CHANNEL0, ticks, true);
    nrfx_timer_clear(&tmrs[m_idx]);
    tos::kern::refresh_interrupts();
}

void timer0::enable() {
    nrfx_timer_enable(&tmrs[m_idx]);
    write_ticks(m_ticks);
    tos::kern::refresh_interrupts();
}

void timer0::disable() {
    nrfx_timer_disable(&tmrs[m_idx]);
    tos::kern::refresh_interrupts();
}

uint32_t timer0::get_counter() const {
    auto ticks =
        std::min(m_ticks, nrfx_timer_capture(&tmrs[m_idx], NRF_TIMER_CC_CHANNEL1));
    return ticks;
}

uint32_t timer0::get_period() const {
    return m_ticks;
}
} // namespace nrf52
} // namespace tos