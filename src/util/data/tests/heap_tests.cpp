#include <tos/data/heap.hpp>
#include <doctest.h>

namespace tos::data::heap {
namespace {
TEST_CASE("Heap push works") {
    heap<int> h(10);
    REQUIRE(h.push(3));
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(3, h.front());
    REQUIRE(h.push(42));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(3, h.front());
    REQUIRE(h.push(1));
    REQUIRE_EQ(3, h.size());
    REQUIRE_EQ(1, h.front());
}

TEST_CASE("Heap kv push works") {
    heap<int, int> h(10);
    REQUIRE(h.push({3, 100}));
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(100, h.front().second);
    REQUIRE(h.push({42, 200}));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(100, h.front().second);
    REQUIRE(h.push({1, 300}));
    REQUIRE_EQ(3, h.size());
    REQUIRE_EQ(300, h.front().second);
}

TEST_CASE("Heap pop works") {
    heap<int> h(100);
    h.push(2);
    h.push(3);
    h.push(1);
    REQUIRE_EQ(1, h.front());
    REQUIRE_EQ(3, h.size());
    h.pop();
    REQUIRE_EQ(2, h.front());
    REQUIRE_EQ(2, h.size());
    h.pop();
    REQUIRE_EQ(3, h.front());
    REQUIRE_EQ(1, h.size());
    h.pop();
    REQUIRE_EQ(0, h.size());
}
}
}