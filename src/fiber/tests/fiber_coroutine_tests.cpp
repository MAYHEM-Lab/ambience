#include "tos/stack_storage.hpp"
#include <doctest.h>
#include <tos/fiber/coroutine.hpp>
#include <tos/fiber/start.hpp>

namespace tos::fiber {
namespace {
tos::Task<int> foo() {
    co_return 42;
}

TEST_CASE("fiber_await works") {
    auto f = owning<>::start(stack_size_t{TOS_DEFAULT_STACK_SIZE}, [](auto& fib) {
        auto val = fiber_await(fib, foo());
        REQUIRE_EQ(42, val);
    });
    f->resume();
}
} // namespace
} // namespace tos::fiber