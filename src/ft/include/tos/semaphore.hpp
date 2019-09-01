#pragma once

#include <chrono>
#include <common/alarm.hpp>
#include <tos/barrier.hpp>
#include <tos/waitable.hpp>

namespace tos {
enum class sem_ret
{
    /**
     * Semaphore down executed successfully
     */
    normal,

    /**
     * Semaphore down timed out
     */
    timeout
};

/**
 * This class implements a Dijkstra style counting semaphore
 *
 * Multiple threads could be holding different units of the
 * semaphore at the same time.
 */
template<class CountT>
class semaphore_base : public non_copy_movable {
public:
    /**
     * Increments the shared counter and wakes up
     * one thread if there's any
     *
     * From ISRs, prefer calling `up_isr`
     */
    void up() noexcept;

    /**
     * Increments the shared counter and wakes up
     * one thread if there's any
     *
     * If called from non-ISR context, the behaviour
     * is undefined due to race conditions
     */
    void up_isr() noexcept;
    // precondition: interrupts must be disabled

    /**
     * Decrements the shared counter and potentially
     * blocks the calling thread if the counter is
     * negative
     */
    void down() & noexcept;

    /**
     * A temporary semaphore can't be blocked on
     * as it would block indefinitely
     */
    void down() && = delete;

    /**
     * Decrements the shared counter and blocks for up
     * to the given amount of time if the counter is
     * negative.
     *
     * Upon timing out, the initial decrement is rolled
     * back.
     *
     * @tparam AlarmT type of the alarm object
     * @param alarm the alarm object
     * @param ms maximum duration to block
     * @return reason for the return
     */
    template<class AlarmT>
    sem_ret down(AlarmT& alarm, std::chrono::milliseconds ms) noexcept;

    /**
     * Initializes a semaphore with the given value
     */
    explicit semaphore_base(CountT n) noexcept
        : m_count(n) {
    }

private:
    CountT m_count;
    waitable m_wait;

    /**
     * Extracts the shared counter value from the given semaphore
     *
     * @param s the semaphore to extract from
     * @return counter value
     */
    friend CountT get_count(const semaphore_base& s) {
        return s.m_count;
    }

    /**
     * Extracts the waiting threads of the given semaphore
     *
     * @param s the semaphore to extract from
     * @return waiting threads
     */
    friend const waitable& get_waiters(const semaphore_base& s) {
        return s.m_wait;
    }

    friend void reset(semaphore_base& s, CountT val) {
        s.m_count = val;
    }

    friend void up_many_isr(semaphore_base& s, CountT n) {
        s.m_count += n;
        s.m_wait.signal_n(n);
    }

    friend void up_many(semaphore_base& s, CountT n) {
        detail::memory_barrier_enter();
        tos::int_guard ig;
        up_many_isr(s, n);
        detail::memory_barrier_exit();
    }

    friend bool try_down_isr(semaphore_base& s) {
        if (s.m_count > 0) {
            --s.m_count;
            return true;
        }
        return false;
    }
};

using semaphore = semaphore_base<int16_t>;
} // namespace tos

namespace tos {
template<class CountT>
inline void semaphore_base<CountT>::up() noexcept {
    detail::memory_barrier_enter();
    tos::int_guard ig;
    up_isr();
    detail::memory_barrier_exit();
}

template<class CountT>
    inline void semaphore_base<CountT>::down() & noexcept {
    detail::memory_barrier_enter();
    tos::int_guard ig;
    --m_count;
    if (m_count < 0) {
        m_wait.wait(ig);
    }
    detail::memory_barrier_exit();
}

template<class CountT>
template<class AlarmT>
sem_ret semaphore_base<CountT>::down(AlarmT& alarm,
                                     std::chrono::milliseconds ms) noexcept {
    detail::memory_barrier_enter();
    tos::int_guard ig;

    auto ret_val = sem_ret::normal;

    --m_count;
    if (m_count >= 0) {
        return ret_val;
    }

    auto wait_handle = m_wait.add(*self());

    auto timeout = [&] {
        // this executes in ISR context
        ret_val = sem_ret::timeout;
        auto& t = m_wait.remove(wait_handle);
        ++m_count;
        make_runnable(t);
    };
    sleeper s{uint16_t(ms.count()), tos::function_ref<void()>(timeout)};
    auto handle = alarm.set_alarm(s);

    kern::suspend_self(ig);

    if (ret_val != sem_ret::timeout) {
        alarm.cancel(handle);
    }

    detail::memory_barrier_exit();
    return ret_val;
}

template<class CountT>
inline void semaphore_base<CountT>::up_isr() noexcept {
    ++m_count;
    m_wait.signal_one();
}
} // namespace tos
