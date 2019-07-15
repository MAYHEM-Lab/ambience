//
// Created by fatih on 6/8/18.
//

#pragma once

#include <stdint.h>
#include <common/timer.hpp>
#include <tos/function_ref.hpp>
#include <common/driver_base.hpp>

namespace tos
{
    namespace nrf52
    {
        class timer0 : public self_pointing<timer0>
        {
        public:
            timer0(int idx);

            void set_frequency(uint16_t hertz);

            void enable();

            void disable();

            uint16_t get_ticks();

            void set_callback(const function_ref<void()>& cb) { m_cb = cb; }

        private:

            void write_ticks(uint32_t cc);

            function_ref<void()> m_cb;
            uint32_t m_ticks;
            int m_idx;
        };
    }

    /**
     * Opens the TIMER0 peripheral on NRF52 devices
     *
     * @note TIMER0 is unavailable while the softdevice is active! Prefer using TIMER1
     * @return TIMER0 peripheral driver
     */
    inline nrf52::timer0 open_impl(devs::timer_t<0>)
    {
        return nrf52::timer0{0};
    }

    inline nrf52::timer0 open_impl(devs::timer_t<1>)
    {
        return nrf52::timer0{1};
    }
}
