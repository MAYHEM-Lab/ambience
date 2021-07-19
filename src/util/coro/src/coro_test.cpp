#include <doctest.h>
#include <tos/coro/meta.hpp>
#include <tos/detail/poll.hpp>
#include <tos/function_ref.hpp>
#include <tos/task.hpp>
#include <tos/coro/when_all.hpp>

static_assert(std::is_same_v<
              int,
              std::remove_reference_t<tos::coro::meta::await_result_t<tos::Task<int>>>>);

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
tos::function_ref<void()> fref{[](void*) {}};

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

Task<int> a() {
    co_return 63;
}

Task<int> b() {
    co_return 42;
}

Task<void> c() {
    auto&& [a_res, b_res] = co_await tos::coro::when_all(a(), b());
    REQUIRE_EQ(63, a_res);
    REQUIRE_EQ(42, b_res);
}

TEST_CASE("when_all works") {
    auto x = tos::coro::make_pollable(c());
    x.run();
}
} // namespace
} // namespace tos