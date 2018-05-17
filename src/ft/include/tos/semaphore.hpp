#pragma once

#include "waitable.hpp"

namespace tos {
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
         * Initializes a semaphore with the given value
         */
        explicit semaphore(int8_t n) noexcept
                :m_count(n)
        { }

        semaphore(semaphore&&) = delete;

    private:
        int8_t m_count;
        waitable m_wait;

        /**
         * Extracts the shared counter value from the given semaphore
         *
         * @param s the semaphore to extract from
         * @return counter value
         */
        friend int8_t get_count(const semaphore& s)
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

    inline void semaphore::up_isr() noexcept
    {
        ++m_count;
        m_wait.signal_one();
    }
}

