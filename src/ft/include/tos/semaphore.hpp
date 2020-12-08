#pragma once

#include <chrono>
#include <common/alarm.hpp>
#include <tos/barrier.hpp>
#include <tos/cancellation_token.hpp>
#include <tos/compiler.hpp>
#include <tos/function_ref.hpp>
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
    timeout,

    cancelled
};

/**
 * This class implements a Dijkstra style counting semaphore
 *
 * Multiple threads could be holding different units of the
 * semaphore at the same time.
 */
template<class CountT>
class semaphore_base : public non_copy_movable {
    static_assert(std::is_signed_v<CountT>, "Underlying semaphore types must be signed!");

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

    sem_ret down(cancellation_token& cancel) noexcept;

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
    volatile CountT m_count;
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
        detail::memory_barrier();
        tos::int_guard ig;
        up_many_isr(s, n);
        detail::memory_barrier();
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
    detail::memory_barrier();
    tos::int_guard ig(__builtin_return_address(0));
    up_isr();
    detail::memory_barrier();
}

template<class CountT>
inline void semaphore_base<CountT>::down() & noexcept {
    detail::memory_barrier();
    tos::int_guard ig(__builtin_return_address(0));
    --m_count;
    if (m_count < 0) {
        m_wait.wait(ig);
    }
    detail::memory_barrier();
}

template<class CountT>
sem_ret semaphore_base<CountT>::down(cancellation_token& cancel) noexcept {
    sem_ret res = sem_ret::normal;
    detail::memory_barrier();
    tos::int_guard ig(__builtin_return_address(0));
    --m_count;
    if (m_count < 0) {
        auto wait_res = m_wait.wait(ig, cancel);
        if (wait_res) {
            res = sem_ret::cancelled;
        }
    }
    detail::memory_barrier();
    return res;
}

template<class CountT>
template<class AlarmT>
sem_ret semaphore_base<CountT>::down(AlarmT& alarm,
                                     std::chrono::milliseconds ms) noexcept {
    cancellation_token token = cancellation_token::system().nest();

    sleeper s{uint16_t(ms.count()), make_canceller(token)};
    auto handle = alarm.set_alarm(s);

    auto ret_val = down(token);

    if (ret_val == sem_ret::normal) {
        alarm.cancel(handle);
        return ret_val;
    }

    return sem_ret::timeout;
}

template<class CountT>
ISR_AVAILABLE inline void semaphore_base<CountT>::up_isr() noexcept {
    ++m_count;
    m_wait.signal_one();
}

inline tos::function_ref<void()> make_semaphore_upper(semaphore& sem) {
    return mem_function_ref<&semaphore::up>(sem);
}
} // namespace tos