#pragma once

#include <tos/fiber/basic_fiber.hpp>
#include <tos/fiber/start.hpp>

namespace tos::fiber {
struct registered_fiber : public basic_fiber<registered_fiber> {
    void on_resume();
    void on_suspend();
    registered_fiber* m_old;
};

registered_fiber* current_fiber();
registered_fiber* current_fiber(registered_fiber& fib);

inline void registered_fiber::on_resume() {
    m_old = current_fiber(*this);
}

inline void registered_fiber::on_suspend() {
    auto self = current_fiber(*m_old);
    // assert(self == this);
}

using registered_owning = owning<registered_fiber>;
} // namespace tos::fiber