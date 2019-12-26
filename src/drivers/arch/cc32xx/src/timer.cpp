//
// Created by fatih on 11/6/19.
//

#include "tos/debug/panic.hpp"
#include "tos/interrupt.hpp"

#include <arch/timer.hpp>
#include <iterator>
#include <ti/drivers/dpl/ClockP.h>

extern "C" {
extern const Timer_Config Timer_config[];
}

namespace tos::cc32xx {
void timer_isr(Timer_Handle handle) {
    tos::int_guard ig;
    auto id = std::distance(&Timer_config[0], static_cast<const Timer_Config*>(handle));
    timer::get(id)->isr();
}
timer::timer(uint8_t timer_num)
    : tracked_driver(timer_num) {
    static auto _ = [] {
        Timer_init();
        return 0;
    }();
    Timer_Params params;
    params.period = 1;
    params.periodUnits = Timer_PERIOD_HZ;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timer_isr;
    m_handle = Timer_open(timer_num, &params);
    if (!m_handle) {
        tos::debug::panic("can't open timer!");
    }
}

uint32_t timer::get_period() const {
    ClockP_FreqHz clockFreq;
    ClockP_getCpuFreq(&clockFreq);
    return clockFreq.lo / m_freq;
}
} // namespace tos::cc32xx