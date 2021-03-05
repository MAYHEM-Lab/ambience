#include <doctest.h>
#include <tos/detail/poll.hpp>
#include <tos/task.hpp>
#include <tos/function_ref.hpp>

namespace tos {
namespace {
Task<int> bar() {
    co_return 21;
}

Task<int> foo() {
    co_return 21 + co_await bar();
}

Task<int> coro() {
    co_return 50 + co_await foo();
}

Task<void> test() {
    auto x = co_await coro();
    REQUIRE_EQ(92, x);
}

TEST_CASE("task<int> works") {
    auto x = tos::coro::make_pollable(test());
    while (!x.run())
        ;
}

bool coro_set = false;
tos::function_ref<void()> fref{[](void*){}};

Task<void> coro_resumer_test() {
    co_await make_coro_resumer(fref);
    coro_set = true;
}

TEST_CASE("make_coro_resumer works") {
    auto x = tos::coro::make_pollable(coro_resumer_test());
    // Let it run and await on the function ref
    x.run();

    // This resumes the above coroutine after the `co_await make_coro_resumer(fref)` line.
    fref();

    REQUIRE(coro_set);
}
} // namespace
} // namespace tos