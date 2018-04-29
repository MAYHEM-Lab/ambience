#pragma once

#include <tos/thread_info.hpp>
#include <tos/scheduler.hpp>
#include <tos/interrupt.hpp>

namespace tos {
    struct waitable
    {
        void wait();

        void add(thread_info& t);

        void signal_all();

        void signal_one();

    private:
        intrusive_list<thread_info> m_waiters;
    };
}

namespace tos {
    inline void waitable::wait()
    {
        add(*self());
        tos::enable_interrupts();
        wait_yield();
    }

    inline void waitable::add(thread_info& t)
    {
        m_waiters.push_back(t);
    }

    inline void waitable::signal_all()
    {
        while (!m_waiters.empty()) {
            signal_one();
        }
    }

    inline void waitable::signal_one()
    {
        if (m_waiters.empty()) return;
        auto front = &m_waiters.front();
        m_waiters.pop_front();
        make_runnable(front);
    }
}

