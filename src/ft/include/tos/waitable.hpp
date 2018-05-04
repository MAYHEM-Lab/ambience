#pragma once

#include <tos/thread_info.hpp>
#include <tos/scheduler.hpp>
#include <tos/interrupt.hpp>

namespace tos {
    struct waitable
    {
        /**
         * Makes the current thread yield and block on this
         * waitable object.
         *
         * Interrupts must be disabled when this function
         * is called. Interrupts will be re-enabled by the
         * scheduler after yielding.
         *
         * If this function is called from a non-task
         * context, the behaviour is undefined.
         */
        void wait();

        /**
         * Wakes all of the threads that are waiting on
         * this object.
         */
        void signal_all();

        /**
         * Wakes zero or one thread from the ones that
         * are waiting on this object.
         */
        void signal_one();

    private:

        void add(thread_info& t);
        intrusive_list<thread_info> m_waiters;
    };
}

namespace tos {
    inline void waitable::wait()
    {
        add(*self());
        //tos::enable_interrupts();
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

