#pragma once

#include "tos/detail/coro.hpp"
#include "tos/stack_storage.hpp"
#include <tos/fiber/basic_fiber.hpp>
#include <tos/fiber/start.hpp>
#include <utility>

namespace tos::fiber {
struct registered_fiber : public basic_fiber<registered_fiber> {
    void on_resume();
    void on_suspend();
    void on_start();
    registered_fiber* m_old;
};

registered_fiber* current_fiber();
registered_fiber* current_fiber(registered_fiber& fib);

inline void registered_fiber::on_start() {
    on_resume();
}

inline void registered_fiber::on_resume() {
    m_old = current_fiber(*this);
}

inline void registered_fiber::on_suspend() {
    [[maybe_unused]] auto self = current_fiber(*m_old);
    // assert(self == this);
}

using registered_owning = owning<registered_fiber>;
} // namespace tos::fiber