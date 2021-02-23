#include <tos/task.hpp>
#include <doctest.h>

namespace tos {
namespace {
task<int> foo() {
    co_return 42;
}

task<int> coro() {
    co_await foo();
}

TEST_CASE("task<int> works") {
    auto x = coro();
}
}
}