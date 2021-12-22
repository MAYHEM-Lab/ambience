#pragma once

#include <tos/intrusive_list.hpp>
#include <tos/job.hpp>

namespace tos {
struct core_waitable {
    using waiter_handle = intrusive_list<job>::iterator_t;

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
     * Wakes up to n threads
     * @param n number of threads to wake
     */
    void signal_n(size_t n);

    /**
     * Places the given tcb to the waiters list of this
     * waitable object.
     *
     * @param t task to place in this waitable
     * @return handle to the task
     */
    waiter_handle add(job& t);

    job& remove(waiter_handle handle);
    job& remove(job& t);

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
    intrusive_list<job> m_waiters;
};
} // namespace tos