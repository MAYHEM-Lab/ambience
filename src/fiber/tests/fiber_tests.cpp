#include "tos/fiber/basic_fiber.hpp"
#include "tos/fiber/start.hpp"
#include "tos/stack_storage.hpp"
#include <doctest.h>
#include <tos/fiber.hpp>

namespace tos::fiber {
namespace {
TEST_CASE("now_owning fiber works") {
    static stack_storage<TOS_DEFAULT_STACK_SIZE> stak;
    int x = 1;
    auto f = non_owning::start(stak, [&](auto& fib) { x = 42; });
    f->resume();
    REQUIRE_EQ(42, x);
}

TEST_CASE("nested resume works") {
    int x = 1;
    auto f1 =
        unique(owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            x = 42;
            fib.suspend();
        }));
    REQUIRE(f1);
    auto f2 = unique(owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE},
                                     [&x, f1 = f1.get()](auto& fib) {
                                         f1->resume();
                                         REQUIRE_EQ(42, x);
                                         x = 45;
                                         fib.suspend();
                                     }));
    REQUIRE(f2);
    f2->resume();
    REQUIRE_EQ(45, x);
}

TEST_CASE("swap_context works") {
    int x = 1;
    auto f1 =
        unique(owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            x = 42;
            fib.suspend();
        }));
    REQUIRE(f1);
    auto f2 =
        unique(owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE},
                               [f1 = f1.get()](auto& fib) { swap_fibers(fib, *f1); }));
    REQUIRE(f2);
    f2->resume();
    REQUIRE_EQ(42, x);
}

TEST_CASE("async fibers work") {
    static stack_storage<TOS_DEFAULT_STACK_SIZE> stak;
    int x = 1;
    auto f = non_owning::start(stak, [&](auto& fib) {
        // Here we test that async fibers start executing immediately, not when they
        // are waited.
        // If the async fibers ran when they were waited, fut2's fiber would run first,
        // and return 42 and set y to 43, first setting x to 42.
        // Then, fut's fiber would run and set x to 43, and make the test fail.
        // However, if async fibers start running immediately, fut's fiber would get 42
        // and fut2's would get 43, satisfying the test.
        int y = 42;
        auto fut = async([&] { return y++; });
        auto fut2 = async([&] { return y++; });
        x = fut2.get(fib);
        x = fut.get(fib);
    });
    f->resume();
    REQUIRE_EQ(42, x);
}
} // namespace
} // namespace tos::fiber