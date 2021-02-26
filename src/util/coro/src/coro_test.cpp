#include <doctest.h>
#include <tos/detail/poll.hpp>
#include <tos/task.hpp>

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
} // namespace
} // namespace tos