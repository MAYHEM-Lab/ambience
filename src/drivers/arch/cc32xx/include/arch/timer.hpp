//
// Created by fatih on 11/6/19.
//

#pragma once

#include <common/driver_base.hpp>
#include <cstdint>
#include <ti/drivers/Timer.h>
#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>

namespace tos::cc32xx {
class timer
    : public self_pointing<timer>
    , public tracked_driver<timer, 3> {
public:
    explicit timer(uint8_t timer_num);

    uint32_t get_counter() const {
        return Timer_getCount(m_handle);
    }

    uint32_t get_period() const;

    void set_frequency(uint16_t hz) {
        m_freq = hz;
        Timer_setPeriod(native_handle(), Timer_PERIOD_HZ, hz);
    }

    void set_callback(tos::function_ref<void()> fn) {
        m_fun = fn;
    }

    void enable() {
        Timer_start(native_handle());
    }

    void disable() {
        Timer_stop(native_handle());
    }

    Timer_Handle native_handle() {
        return m_handle;
    }

    ~timer() {
        Timer_close(native_handle());
    }

private:
    Timer_Handle m_handle;
    uint32_t m_freq;
    tos::function_ref<void()> m_fun{[](void*) {}};
};
} // namespace tos::cc32xx