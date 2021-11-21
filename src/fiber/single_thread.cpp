#include <tos/fiber/this_fiber.hpp>

namespace tos::fiber {
namespace {
registered_fiber* cur_fiber;
}

registered_fiber* current_fiber() {
    return cur_fiber;
}

registered_fiber* current_fiber(registered_fiber& fib) {
    return std::exchange(cur_fiber, &fib);
}
}