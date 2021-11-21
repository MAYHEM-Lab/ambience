#pragma once

#include <tos/fiber/basic_fiber.hpp>
#include <tos/fiber/start.hpp>

namespace tos::fiber {
struct registered_fiber : public basic_fiber<registered_fiber> {
    void on_resume();
};

registered_fiber* current_fiber();
void current_fiber(registered_fiber& fib);

inline void registered_fiber::on_resume() {
    current_fiber(*this);
}

using registered_owning = owning<registered_fiber>;
} // namespace tos::fiber