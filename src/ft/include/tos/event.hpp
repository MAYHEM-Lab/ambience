#pragma once

#include <tos/barrier.hpp>
#include <tos/waitable.hpp>

namespace tos {

/**
 * Events are very barebones synchronisation primitives.
 *
 * They are susceptible to the lost wake up problem
 */
class event : public non_copy_movable {
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

private:
    waitable m_wait;
    friend waitable& get_waiters(event& e) {
        return e.m_wait;
    }
};
} // namespace tos

namespace tos {
inline void event::fire() noexcept {
    detail::memory_barrier();
    tos::int_guard ig;
    fire_isr();
    detail::memory_barrier();
}

inline void event::wait() noexcept {
    detail::memory_barrier();
    tos::int_guard ig;
    m_wait.wait(ig);
    detail::memory_barrier();
}

inline void event::fire_isr() noexcept {
    detail::memory_barrier();
    m_wait.signal_all();
    detail::memory_barrier();
}
} // namespace tos
