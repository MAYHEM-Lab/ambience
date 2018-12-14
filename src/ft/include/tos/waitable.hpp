#pragma once

#include <tos/ft.hpp>
#include <tos/scheduler.hpp>
#include <tos/interrupt.hpp>
#include <tos/debug.hpp>

namespace tos {
    struct waitable
    {
        using waiter_handle = intrusive_list<kern::tcb>::iterator_t;

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

        /**
         * Places the given tcb to the waiters list of this
         * waitable object.
         *
         * @param t task to place in this waitable
         * @return handle to the task
         */
        waiter_handle add(kern::tcb& t);

        kern::tcb& remove(waiter_handle handle);

        /**
         * Number of tasks in this waitable
         *
         * @return number of tasks
         */
        size_t size() const {
            return m_waiters.size();
        }

        bool empty() const {
            return m_waiters.empty();
        }

    private:

        intrusive_list<kern::tcb> m_waiters;
    };
}

namespace tos {
    inline void waitable::wait()
    {
        if (self() == nullptr)
        {
            kern::fatal("wait called from non thread ctx!");
        }
        add(*self());
        kern::suspend_self();
    }

    inline auto waitable::add(kern::tcb& t) -> waiter_handle
    {
        return m_waiters.push_back(t);
    }

    inline kern::tcb& waitable::remove(waitable::waiter_handle handle) {
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

