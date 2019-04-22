//
// Created by fatih on 5/8/18.
//

#pragma once

#include <cstdint>
#include <tos/intrusive_list.hpp>
#include <tos/event.hpp>
#include <chrono>
#include <tos/function_ref.hpp>
#include <tos/devices.hpp>
#include <common/driver_base.hpp>

namespace tos
{
    struct sleeper
            : list_node<sleeper>
    {
        explicit sleeper(uint16_t ticks, const function_ref<void()>& fun)
            : sleep_ticks(ticks), m_fun(fun) {}
    private:

        uint16_t sleep_ticks;
        function_ref<void()> m_fun;

        template <class T>
        friend class alarm;
    };

    /**
     * This class template implements software alarm functionality over a
     * single hardware timer.
     *
     * The alarm abstraction allows threads to block on timers, also called
     * sleeping.
     *
     * The basic feature of the alarm is that it multiplexes a single
     * hardware timer to support multiple timer events.
     *
     * @tparam T type of the base timer
     */
    template <class T>
    class alarm : public self_pointing<alarm<T>>
    {
    public:
        using alarm_handle = intrusive_list<sleeper>::iterator_t;

        explicit alarm(T& t) : m_timer(&t) {}

        /**
         * The calling thread will be blocked for the given amount
         * of time.
         *
         * @param dur duration to block for
         */
        void sleep_for(std::chrono::milliseconds dur)
        {
          event ev;
          auto fun = [&ev]{
            ev.fire_isr();
          };
          sleeper s { uint16_t(dur.count()), fun };
          set_alarm(s);
          ev.wait();
        }

        auto set_alarm(sleeper& s) -> alarm_handle
        {
            tos::int_guard no_int;

            auto it = m_sleepers.begin();
            while (it != m_sleepers.end() && it->sleep_ticks < s.sleep_ticks)
            {
              s.sleep_ticks -= it->sleep_ticks;
              ++it;
            }
            if (it != m_sleepers.end())
            {
              it->sleep_ticks -= s.sleep_ticks;
            }
            else
            {
              start();
            }
            return m_sleepers.insert(it, s);
        }

        void cancel(alarm_handle it)
        {
            tos::int_guard no_int;

            auto next = it;
            ++next;
            if (next != m_sleepers.end())
            {
                next->sleep_ticks += it->sleep_ticks;
            }
            m_sleepers.erase(it);

            if (m_sleepers.empty())
            {
                stop();
            }
        }

    private:

        void start()
        {
            (*m_timer)->set_callback({[](void* data){
                static_cast<alarm*>(data)->tick_handler();
            }, this});
            (*m_timer)->set_frequency(1000 / m_period);
            (*m_timer)->enable();
        }

        void stop()
        {
            (*m_timer)->disable();
        }

        void tick_handler()
        {
            sleeper& front = m_sleepers.front();
            auto prev = front.sleep_ticks;
            front.sleep_ticks -= m_period;
            if (front.sleep_ticks == 0
                || front.sleep_ticks > prev) // may underflow
            {
                m_sleepers.pop_front();
                front.m_fun();
                if (m_sleepers.empty())
                {
                    stop();
                }
            }
        }

        int m_period = 1; // in milliseconds
        intrusive_list<sleeper> m_sleepers;
        T* m_timer;
    };

    namespace devs
    {
        using alarm_t = tos::dev<struct alarm_t_, 0>;
        static constexpr alarm_t alarm{};
    } // namespace devs

    template <class T>
    auto open_impl(devs::alarm_t, T& tmr)
    {
        return alarm<std::remove_reference_t<T>>{tmr};
    }
} // namespace tos