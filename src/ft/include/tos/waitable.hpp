#pragma once

#include <tos/cancellation_token.hpp>
#include <tos/debug/assert.hpp>
#include <tos/debug/debug.hpp>
#include <tos/detail/coro.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

namespace tos {
struct waitable {
    using waiter_handle = intrusive_list<job>::iterator_t;

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

    bool wait(const int_guard&, cancellation_token& cancel);

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

    auto operator co_await() {
        struct awaiter : job {
            awaiter(context& ctx, waitable& w)
                : job(ctx)
                , m_waitable{&w} {
            }

            bool await_ready() const noexcept {
                return false;
            }

            void await_suspend(std::coroutine_handle<> coro) {
                m_cont = coro;
                m_waitable->add(*this);
            }

            void await_resume() {
            }

            void operator()() override {
                m_cont.resume();
            }

            awaiter(awaiter&&) = delete;
            awaiter(const awaiter&) = delete;

            waitable* m_waitable;
            std::coroutine_handle<> m_cont;
        };

        return awaiter{current_context(), *this};
    }

private:
    intrusive_list<job> m_waiters;
};
} // namespace tos

namespace tos {
inline void waitable::wait(const int_guard& ni) {
    Assert(self() && "wait must be called from a thread!");
    add(*self());
    kern::suspend_self(ni);
}

inline bool waitable::wait(const int_guard& ig, cancellation_token& cancel) {
    bool res = false;

    auto wait_handle = add(*self());

    auto cancel_cb = [&] {
        res = true;
        auto& t = remove(wait_handle);
        kern::make_runnable(t);
    };

    cancel.set_cancel_callback(tos::function_ref<void()>(cancel_cb));

    kern::suspend_self(ig);

    return res;
}

inline auto waitable::add(job& t) -> waiter_handle {
    return m_waiters.push_back(t);
}

inline job& waitable::remove(waitable::waiter_handle handle) {
    auto& ret = *handle;
    m_waiters.erase(handle);
    return ret;
}

inline job& waitable::remove(job& t) {
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
    kern::make_runnable(front);
}

inline void waitable::signal_n(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        signal_one();
    }
}
} // namespace tos
