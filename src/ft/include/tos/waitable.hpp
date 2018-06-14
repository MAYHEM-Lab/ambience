#pragma once

#include <tos/ft.hpp>
#include <tos/scheduler.hpp>
#include <tos/interrupt.hpp>

namespace tos {
    struct waitable
    {
        using waiter_handle = intrusive_list<tcb>::iterator_t;

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

        waiter_handle add(tcb& t);
        tcb& remove(waiter_handle);

    private:

        intrusive_list<tcb> m_waiters;
    };
}

namespace tos {
    inline void waitable::wait()
    {
        add(*self());
        suspend_self();
    }

    inline auto waitable::add(tcb& t) -> waiter_handle
    {
        return m_waiters.push_back(t);
    }

    inline tcb& waitable::remove(waitable::waiter_handle handle) {
        auto& ret = *handle;
        m_waiters.erase(handle);
        return ret;
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
        auto& front = m_waiters.front();
        m_waiters.pop_front();
        make_runnable(front);
    }
}

