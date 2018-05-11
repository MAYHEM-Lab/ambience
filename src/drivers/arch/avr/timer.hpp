//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stdint.h>
#include <drivers/common/timer.hpp>
#include <tos/function_ref.hpp>

namespace tos {
    namespace avr {
        class timer1 {
        public:
            static void set_frequency(uint16_t hertz);

            static void enable();

            static void disable();

            static uint16_t get_ticks();

            static void set_callback(const function_ref<void()>&);
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

    inline avr::timer1 *open_impl(devs::timer_t<1>) {
        return nullptr;
    }
}