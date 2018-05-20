//
// Created by fatih on 5/8/18.
//

#pragma once

#include <stdint.h>
#include <tos/intrusive_list.hpp>
#include <tos/event.hpp>

namespace tos
{
    struct sleeper
            : list_node<sleeper>
    {
        uint16_t sleep_ticks;
        event ev;
        explicit sleeper(uint16_t ticks) : sleep_ticks(ticks) {}
    };

    struct milliseconds
    {
        uint64_t val;
    };

    template <class T>
    class alarm
    {
    public:
        explicit alarm(T& t) : m_timer(&t) {
            m_timer->set_callback({[](void* data){
                static_cast<alarm*>(data)->tick_handler();
            }, this});
        }

        void sleep_for(milliseconds dur)
        {
            sleeper s { uint16_t(dur.val) };
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
                m_sleepers.insert(it, s);
            }
            s.ev.wait();
        }


        constexpr milliseconds min_resolution() const { return { 1 }; }

    private:

        void start()
        {
            m_timer->set_frequency(1000);
            m_timer->enable();
        }

        void stop()
        {
            m_timer->disable();
        }

        void tick_handler()
        {
            auto& front = m_sleepers.front();
            front.sleep_ticks--;
            if (front.sleep_ticks == 0)
            {
                m_sleepers.pop_front();
                front.ev.fire();
            }
            if (m_sleepers.empty())
            {
                stop();
            }
        }

        intrusive_list<sleeper> m_sleepers;
        T* m_timer;
    };
}