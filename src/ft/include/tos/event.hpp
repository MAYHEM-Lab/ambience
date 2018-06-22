#pragma once

#include <tos/waitable.hpp>

namespace tos {
    class event
    {
    public:
        /**
         * Fires the event. If there are any threads
         * blocked on this event, all of them will be
         * awoken.
         *
         * Prefer the `fire_isr` function within ISR
         * contexts.
         */
        void fire() noexcept;

        /**
         * Fires the event. If there are any threads
         * blocked on this event, all of them will be
         * awoken.
         *
         * If called from a non-ISR context, behaviour
         * is undefined.
         */
        void fire_isr() noexcept;

        /**
         * Unconditionally suspends the current thread
         * to wait on this event.
         */
        void wait() noexcept;

        explicit event() noexcept {}
        event(event&&) = delete;

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

