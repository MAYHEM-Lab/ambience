#include <doctest.h>
#include <tos/memory/free_list.hpp>

namespace tos::memory {
namespace {
TEST_CASE("free list allocation works") {
    std::array<uint8_t, 128> store;
    free_list alloc(store);
    REQUIRE_EQ(128, alloc.available_memory());

    auto ptr = alloc.allocate(4);
    REQUIRE_NE(nullptr, ptr);

    auto ptr2 = alloc.allocate(4);
    REQUIRE_NE(nullptr, ptr2);

    REQUIRE(ptr != ptr2);
}

TEST_CASE("free list free works") {
    std::array<uint8_t, 64> store;
    free_list alloc(store);

    for (int i = 0; i < 128; ++i) {
        auto ptr = alloc.allocate(4);
        REQUIRE_NE(nullptr, ptr);
        alloc.free(ptr);
    }
}
} // namespace
} // namespace tos::memory