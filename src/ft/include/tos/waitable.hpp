#pragma once

#include <tos/debug/debug.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

namespace tos {
struct waitable {
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
    void wait(const int_guard&);

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
    waiter_handle add(kern::tcb& t);

    kern::tcb& remove(waiter_handle handle);
    kern::tcb& remove(kern::tcb& t);

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
} // namespace tos

namespace tos {
inline void waitable::wait(const int_guard& ni) {
    if (self() == nullptr) {
        debug::panic("wait called from non thread ctx!");
    }
    add(*self());
    kern::suspend_self(ni);
}

inline auto waitable::add(kern::tcb& t) -> waiter_handle {
    return m_waiters.push_back(t);
}

inline kern::tcb& waitable::remove(waitable::waiter_handle handle) {
    auto& ret = *handle;
    m_waiters.erase(handle);
    return ret;
}

inline kern::tcb& waitable::remove(kern::tcb& t) {
    return remove(std::find_if(
        m_waiters.begin(), m_waiters.end(), [&t](auto& tcb) { return &t == &tcb; }));
}

inline void waitable::signal_all() {
    while (!m_waiters.empty()) {
        signal_one();
    }
}

inline void waitable::signal_one() {
    if (m_waiters.empty())
        return;
    auto& front = m_waiters.front();
    m_waiters.pop_front();
    make_runnable(front);
}

inline void waitable::signal_n(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        signal_one();
    }
}
} // namespace tos
