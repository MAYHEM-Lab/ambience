//
// Created by fatih on 6/8/18.
//

#pragma once

#include <stdint.h>
#include <drivers/common/timer.hpp>
#include <tos/function_ref.hpp>
#include <drivers/common/driver_base.hpp>

namespace tos
{
    namespace nrf52
    {
        class timer0 : public self_pointing<timer0>
        {
        public:
            timer0();

            void set_frequency(uint16_t hertz);

            void enable();

            void disable();

            uint16_t get_ticks();

            void set_callback(const function_ref<void()>& cb) { m_cb = cb; }

        private:

            void write_ticks(uint32_t cc);

            function_ref<void()> m_cb;
            uint32_t m_ticks;
        };
    }

    inline nrf52::timer0 open_impl(devs::timer_t<0>)
    {
        return nrf52::timer0{};
    }
}
