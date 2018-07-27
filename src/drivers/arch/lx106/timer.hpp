//
// Created by fatih on 6/26/18.
//

#pragma once

#include <stdint.h>
#include <tos/function_ref.hpp>
#include <drivers/common/timer.hpp>

extern "C"
{
#include <osapi.h>
}

namespace tos
{
    namespace esp82
    {
        class timer
        {
        public:
            timer();

            void set_frequency(uint16_t hertz);

            void enable();

            void disable();

            uint16_t get_ticks();

            void set_callback(const function_ref<void()>& cb);

            timer* operator->(){return this;}
            timer& operator*(){return *this;}
        private:

            function_ref<void()> m_cb;
            uint16_t m_freq;

            os_timer_t m_timer;
        };
    }

    inline esp82::timer open_impl(devs::timer_t<0>)
    {
        return esp82::timer{};
    }
}
