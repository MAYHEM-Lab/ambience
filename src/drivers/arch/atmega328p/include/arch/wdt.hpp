//
// Created by fatih on 12/13/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <tos/function_ref.hpp>
#include <tos/event.hpp>
#include "../../../../../../../../../usr/lib/avr/include/avr/wdt.h"

extern "C" void WDT_vect();

namespace tos
{
namespace avr
{
    enum class wdt_times : uint8_t
    {
        to_15ms = WDTO_15MS,
        to_30ms = WDTO_30MS,
        to_1s = WDTO_1S,
        to_8s = WDTO_8S
    };

    extern tos::function_ref<void()> wdt_handler;

    class wdt
            : public tracked_driver<wdt, 1>
            , public self_pointing<wdt>
    {
    public:
        wdt() : tracked_driver(0) {
            wdt_handler = tos::function_ref<void()>([](void* self){
                auto ptr = static_cast<wdt*>(self);
                ptr->wdt_interrupt();
            }, this);
        }

        void wait(wdt_times time)
        {
            wdt_enable(uint8_t (time));
            WDTCSR |= (1 << WDIE);
            ev.wait();
        }
    private:

        void wdt_interrupt()
        {
            ev.fire_isr();
            wdt_disable();
        }
        tos::event ev;
    };

    class wdt_resetter
    {
    public:
        wdt_resetter(){
            wdt_handler = tos::function_ref<void()>([](void* self){
                auto ptr = static_cast<wdt_resetter*>(self);
                ptr->wdt_interrupt();
            }, this);
        }

        void start(std::chrono::seconds secs)
        {
            m_total = secs;
            m_remaining = secs;
            wdt_enable(WDTO_1S);
            WDTCSR |= (1 << WDIE);
        }

        void reset()
        {
            tos::int_guard ig;
            m_remaining = m_total;
            wdt_reset();
        }

        void disable()
        {
            wdt_disable();
        }

        ~wdt_resetter()
        {
            disable();
        }

    private:

        void wdt_interrupt()
        {
            using namespace std::chrono_literals;
            m_remaining -= 1s;

            if (m_remaining > 1s)
            {
                WDTCSR |= (1 << WDIE);
            }
            else
            {
                // if wdt isn't disabled in a second, we'll reset
            }
        }

        std::chrono::seconds m_total;
        std::chrono::seconds m_remaining;
    };
} // namespace avr
} // namespace tos