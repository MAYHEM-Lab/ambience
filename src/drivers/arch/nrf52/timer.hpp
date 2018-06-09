//
// Created by fatih on 6/8/18.
//

#pragma once

#include <stdint.h>
#include <drivers/common/timer.hpp>
#include <tos/function_ref.hpp>

namespace tos
{
    namespace arm
    {
        class timer0
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
}
