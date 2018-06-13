//
// Created by fatih on 5/8/18.
//

#pragma once

#include <stdint.h>
#include <tos/intrusive_list.hpp>
#include <tos/event.hpp>
#include <tos/chrono.hpp>
#include <tos/function_ref.hpp>
#include <tos/devices.hpp>

namespace tos
{
    struct sleeper
            : list_node<sleeper>
    {
        explicit sleeper(uint16_t ticks, const function_ref<void()>& fun) : sleep_ticks(ticks), fun(fun) {}

    private:
        uint16_t sleep_ticks;
        function_ref<void()> fun;

        template <class T>
        friend class alarm;
    };

    template <class T>
    class alarm
    {
    public:
        using alarm_handle = intrusive_list<sleeper>::iterator_t;

        explicit alarm(T& t) : m_timer(&t) {
        }

        void sleep_for(milliseconds dur)
        {
            event ev;
            auto fun = [&ev]{
                ev.fire_isr();
            };
            sleeper s { uint16_t(dur.val), fun };
            set_alarm(s);
            ev.wait();
        }

        auto set_alarm(sleeper& s) -> alarm_handle
        {
            tos::int_guard no_int;
            auto it = m_sleepers.begin();
            while (it != m_sleepers.end() && it->sleep_ticks < s.sleep_ticks)
            {
                ++it;
            }
            if (it != m_sleepers.end())
            {
                s.sleep_ticks -= it->sleep_ticks;
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

        constexpr milliseconds min_resolution() const { return { 1 }; }

    private:

        void start()
        {
            m_timer->set_callback({[](void* data){
                static_cast<alarm*>(data)->tick_handler();
            }, this});
            m_timer->set_frequency(1000);
            m_timer->enable();
        }

        void stop()
        {
            m_timer->disable();
        }

        void tick_handler()
        {
            sleeper& front = m_sleepers.front();
            front.sleep_ticks--;
            if (front.sleep_ticks == 0)
            {
                m_sleepers.pop_front();
                front.fun();
                if (m_sleepers.empty())
                {
                    stop();
                }
            }
        }

        intrusive_list<sleeper> m_sleepers;
        T* m_timer;
    };

    namespace devs
    {
        using alarm_t = tos::dev<struct alarm_t_, 0>;
        static constexpr alarm_t alarm{};
    }

    template <class T>
    auto open_impl(devs::alarm_t, T& tmr)
    {
        return alarm<tos::remove_reference_t<T>>{tmr};
    }
}