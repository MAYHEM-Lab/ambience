#include "tos/fiber/basic_fiber.hpp"
#include "tos/fiber/start.hpp"
#include "tos/stack_storage.hpp"
#include <doctest.h>
#include <tos/fiber.hpp>
#include <tos/fiber/this_fiber.hpp>

namespace tos::fiber {
namespace {
TEST_CASE("now_owning fiber works") {
    static stack_storage<TOS_DEFAULT_STACK_SIZE> stak;
    int x = 1;
    auto f = non_owning::start(stak, [&](auto& fib) { x = 42; });
    f->resume();
    REQUIRE_EQ(42, x);
}

TEST_CASE("swap_context works") {
    int x = 1;
    auto f1 = unique(owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
        x = 42;
        fib.suspend();
    }));
    REQUIRE(f1);
    auto f2 = unique(
        owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE},
                      [f1 = f1.get()](auto& fib) { swap_fibers(fib, *f1, [] {}); }));
    REQUIRE(f2);
    f2->resume();
    REQUIRE_EQ(42, x);
}

TEST_CASE("registered_fiber works") {
    int x = 1;
    auto f = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, &tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
        }));
    f->resume();
    f->resume();
    REQUIRE_EQ(42, x);
}
} // namespace
} // namespace tos::fiber