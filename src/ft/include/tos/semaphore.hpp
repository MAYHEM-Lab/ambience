#pragma once

#include "waitable.hpp"

namespace tos {
    class semaphore
    {
    public:
        void up() noexcept;

        void down() noexcept;

        explicit semaphore(int8_t n) noexcept
                :m_count(n)
        { }

    private:
        int8_t m_count;
        waitable m_wait;

        friend int8_t get_count(const semaphore& s)
        {
            return s.m_count;
        }
    };

}

namespace tos {
    inline void semaphore::up() noexcept
    {
        tos::int_guard ig;
        ++m_count;
        m_wait.signal_one();
    }

    inline void semaphore::down() noexcept
    {
        tos::disable_interrupts();
        --m_count;
        if (m_count<0) {
            m_wait.wait();
        }
        else {
            tos::enable_interrupts();
        }
    }
}

