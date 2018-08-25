#pragma once

#include <tos/chrono.hpp>
#include <tos/waitable.hpp>
#include <drivers/common/alarm.hpp>

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
     */
    class semaphore
    {
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
        void down() noexcept;

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
        template <class AlarmT>
        sem_ret down(AlarmT& alarm, milliseconds ms) noexcept;

        /**
         * Initializes a semaphore with the given value
         */
        explicit semaphore(int16_t n) noexcept
                :m_count(n)
        { }

        semaphore(semaphore&&) = delete;

    private:
        int16_t m_count;
        waitable m_wait;

        /**
         * Extracts the shared counter value from the given semaphore
         *
         * @param s the semaphore to extract from
         * @return counter value
         */
        friend int16_t get_count(const semaphore& s)
        {
            return s.m_count;
        }

        /**
         * Extracts the waiting threads of the given semaphore
         *
         * @param s the semaphore to extract from
         * @return waiting threads
         */
        friend const waitable& get_waiters(const semaphore& s)
        {
            return s.m_wait;
        }

        friend bool try_down_isr(semaphore& s)
        {
            if (s.m_count > 0)
            {
                --s.m_count;
                return true;
            }
            return false;
        }
    };
}

namespace tos {
    inline void semaphore::up() noexcept
    {
        tos::int_guard ig;
        up_isr();
    }

    inline void semaphore::down() noexcept
    {
        tos::int_guard ig;
        --m_count;
        if (m_count < 0) {
            m_wait.wait();
        }
    }

    template<class AlarmT>
    sem_ret semaphore::down(AlarmT &alarm, milliseconds ms) noexcept {
        tos::int_guard ig;

        auto ret_val = sem_ret::normal;

        --m_count;
        if (m_count >= 0) {
            return ret_val;
        }

        auto wait_handle = m_wait.add(*self());

        auto timeout = [&]{
            // this executes in ISR context
            ret_val = sem_ret::timeout;
            auto& t = m_wait.remove(wait_handle);
            ++m_count;
            make_runnable(t);
        };
        sleeper s { uint16_t(ms.val), timeout };
        auto handle = alarm.set_alarm(s);

        kern::suspend_self();

        if (ret_val != sem_ret::timeout)
        {
            alarm.cancel(handle);
        }
        return ret_val;
    }

    inline void semaphore::up_isr() noexcept
    {
        ++m_count;
        m_wait.signal_one();
    }
}

