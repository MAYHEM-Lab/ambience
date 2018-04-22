//
// Created by fatih on 4/16/18.
//

#include <avr/interrupt.h>
#include <avr/io.h>
#include <timer.hpp>
#include <tos/event.hpp>
#include <util/atomic.h>
#include <tos/semaphore.hpp>

static tos::event block;
static uint16_t cnt = 0;
static void(*tcb)(void*);
static void* userdata;

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

        void timer1::set_callback(void (*cb)(void*), void* userdata)
        {
            tcb = cb;
            ::userdata = userdata;
        }

        void timer1::enable() {
            PRR &= ~(1 << PRTIM1);
        }

        void timer1::disable() {
            PRR |= (1 << PRTIM1);
        }

        void timer1::block() {
            ::block.wait();
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
    block.fire();
    if (tcb) tcb(userdata);
    TCNT1 = cnt;
}
