#pragma once

#include <tos/waitable.hpp>

namespace tos {
    class event
    {
    public:
        void fire() noexcept;

        void wait() noexcept;

        explicit event() noexcept
        { }

        void fire_isr() noexcept
        {
            m_wait.signal_all();
        }

    private:
        waitable m_wait;
    };
}

namespace tos {
    inline void event::fire() noexcept
    {
        tos::int_guard ig;
        m_wait.signal_all();
    }

    inline void event::wait() noexcept
    {
        tos::disable_interrupts();
        m_wait.wait();
    }
}

