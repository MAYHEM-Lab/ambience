//
// Created by fatih on 11/6/19.
//

#include <arch/timer.hpp>
#include <iterator>
#include <ti/drivers/dpl/ClockP.h>

extern "C" {
extern const Timer_Config Timer_config[];
}

namespace tos::cc32xx {
timer::timer(uint8_t timer_num)
    : tracked_driver(timer_num) {
    static auto _ = []{
      Timer_init();
      return 0;
    }();
    Timer_Params params;
    params.period = 1;
    params.periodUnits = Timer_PERIOD_HZ;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = [](Timer_Handle handle) {
        auto id =
            std::distance(&Timer_config[0], static_cast<const Timer_Config*>(handle));
        timer::get(id)->m_fun();
    };
    m_handle = Timer_open(timer_num, &params);
}

uint32_t timer::get_period() const {
    ClockP_FreqHz clockFreq;
    ClockP_getCpuFreq(&clockFreq);
    return clockFreq.lo / m_freq;
}
} // namespace tos::cc32xx