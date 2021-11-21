#include "tos/stack_storage.hpp"
#include <doctest.h>
#include <tos/fiber.hpp>
#include <tos/fiber/this_fiber.hpp>

namespace tos::fiber {
namespace {
TEST_CASE("registered_fiber works") {
    int x = 1;
    auto f = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
        }));
    f->resume();
    f->resume();
    REQUIRE_EQ(42, x);
}

TEST_CASE("registered_fiber works with 2 fibers") {
    int x = 1;
    auto f1 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
        }));
    auto f2 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 45;
            fib.suspend();
        }));
    f1->resume();
    f2->resume();

    f1->resume();
    REQUIRE_EQ(42, x);
    f2->resume();
    REQUIRE_EQ(45, x);
}

TEST_CASE("registered_fiber works with 2 fibers and multiple suspends") {
    int x = 1;
    auto f1 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 48;
            fib.suspend();
        }));
    auto f2 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 45;
            fib.suspend();
        }));
    f1->resume();
    f2->resume();

    f1->resume();
    REQUIRE_EQ(42, x);
    f2->resume();
    REQUIRE_EQ(45, x);
    f1->resume();
    REQUIRE_EQ(48, x);
}

TEST_CASE("registered_fiber works with nested resumes") {
    int x = 1;
    auto f1 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
        }));
    auto f2 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            f1->resume();
            REQUIRE_EQ(42, x);
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 45;
            fib.suspend();
        }));
    f1->resume();
    f2->resume();

    f2->resume();
    REQUIRE_EQ(45, x);
}

TEST_CASE("registered_fiber works with swap_fibers") {
    int x = 1;
    REQUIRE_EQ(nullptr, tos::fiber::current_fiber());
    auto f1 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 42;
            fib.suspend();
        }));
    auto f2 = unique(tos::fiber::registered_owning::start(
        stack_size_t{TOS_DEFAULT_STACK_SIZE}, [&](auto& fib) {
            fib.suspend();
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            swap_fibers(fib, *f1, []{});
            REQUIRE_EQ(42, x);
            REQUIRE_EQ(&fib, tos::fiber::current_fiber());
            x = 45;
            fib.suspend();
        }));
    f1->resume();
    f2->resume();

    f2->resume();
    REQUIRE_EQ(nullptr, tos::fiber::current_fiber());
    REQUIRE_EQ(42, x);
    f2->resume();
    REQUIRE_EQ(nullptr, tos::fiber::current_fiber());
    REQUIRE_EQ(45, x);
}
} // namespace
} // namespace tos::fiber