//
// Created by fatih on 4/16/18.
//

#include <avr/interrupt.h>
#include <avr/io.h>
#include <timer.hpp>
#include <tos/event.hpp>
#include <util/atomic.h>
#include <tos/semaphore.hpp>

static uint16_t cnt = 0;
static tos::function_ref<void()> tmr_cb {+[](void*){}, nullptr};

namespace tos
{
    namespace avr
    {
        void timer1::set_frequency(uint16_t hertz) {
            const uint32_t raw_freq = F_CPU / 8;

            auto cnt = uint16_t(65563 - raw_freq / hertz);
            ::cnt = cnt;

            TCCR1A = 0;
            TCCR1B = (1 << CS11);
            TIMSK1 |= (1 << TOIE1);
            TCNT1 = cnt;
        }

        void timer1::set_callback(const function_ref<void()>& cb)
        {
            tmr_cb = cb;
        }

        void timer1::enable() {
            tos::kern::busy();
            PRR &= ~(1 << PRTIM1);
        }

        void timer1::disable() {
            PRR |= (1 << PRTIM1);
            tos::kern::unbusy();
        }

        timer1::~timer1()
        {
            if ((PRR & (1 << PRTIM1)) == 0)
            {
                disable();
            }
        }

        uint16_t timer1::get_ticks() {
            // TODO: should this be atomic?

            uint16_t res;
            //ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                res = TCNT1;
            }
            return res;
        }
    }
}

ISR(TIMER1_OVF_vect)
{
    TCNT1 = cnt;
    tmr_cb();
}
