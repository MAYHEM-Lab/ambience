#include <doctest.h>
#include <tos/data/mutable_heap.hpp>

namespace tos::data::heap {
namespace {
TEST_CASE("Mutable heap push works") {
    mut_heap<int> h(100);
    REQUIRE_EQ(0, h.size());
    auto ten_handle = h.push(10);
    REQUIRE(ten_handle);
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(5));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(5, h.front());
    h.pop();
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(3));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(3, h.front());
}
TEST_CASE("Mutable heap decrease works") {
    mut_heap<int> h(100);
    REQUIRE_EQ(0, h.size());
    auto ten_handle = h.push(10);
    REQUIRE(ten_handle);
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(5));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(5, h.front());
    h.pop();
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(3));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(3, h.front());
    h.decrease(*ten_handle, 1);
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(1, h.front());
}
TEST_CASE("Mutable heap increase works") {
    mut_heap<int> h(100);
    REQUIRE_EQ(0, h.size());
    auto ten_handle = h.push(10);
    REQUIRE(ten_handle);
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(5));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(5, h.front());
    h.pop();
    REQUIRE_EQ(1, h.size());
    REQUIRE_EQ(10, h.front());
    REQUIRE(h.push(30));
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(10, h.front());
    h.increase(*ten_handle, 40);
    REQUIRE_EQ(2, h.size());
    REQUIRE_EQ(30, h.front());
}
} // namespace
} // namespace tos::data::heap