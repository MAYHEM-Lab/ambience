#pragma once

#include <tos/waitable.hpp>

namespace tos {
    class event
    {
    public:
        void fire() noexcept;

        void fire_isr() noexcept;

        void wait() noexcept;

        explicit event() noexcept
        { }

    private:
        waitable m_wait;
    };
}

namespace tos {
    inline void event::fire() noexcept
    {
        tos::int_guard ig;
        fire_isr();
    }

    inline void event::wait() noexcept
    {
        tos::int_guard ig;
        m_wait.wait();
    }

    inline void event::fire_isr() noexcept
    {
        m_wait.signal_all();
    }
}

