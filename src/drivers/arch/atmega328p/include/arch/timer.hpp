//
// Created by fatih on 4/16/18.
//

#pragma once

#include "../../../../../../../../../usr/lib/gcc/avr/5.4.0/include/stdint.h"
#include <common/timer.hpp>
#include <tos/function_ref.hpp>
#include <common/driver_base.hpp>

namespace tos {
    namespace avr {
        class timer1 : public self_pointing<timer1>
        {
        public:
            static void set_frequency(uint16_t hertz);

            static void enable();

            static void disable();

            static void set_callback(const function_ref<void()>&);

            timer1() = default;
            timer1(const timer1&) = delete;
            timer1(timer1&& tmr) noexcept { tmr.m_disable = false; }
            ~timer1();

        private:
            bool m_disable{true};
        };
        class timer0 {
          public:
            static void set_frequency(uint16_t hertz);

            static void enable();

            static void disable();

            static uint8_t get_ticks();

            static void block();

            static void set_callback(void(*)(void*), void*);
        };
    }

    inline avr::timer1 open_impl(devs::timer_t<1>) {
        return {};
    }
}