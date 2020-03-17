#include <doctest.h>
#include <tos/memory/free_list.hpp>
#include <list>

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
    std::array<uint8_t, 256> store;
    free_list alloc(store);

    for (int i = 0; i < 128; ++i) {
        auto ptr = alloc.allocate(i);
        REQUIRE_NE(nullptr, ptr);
        alloc.free(ptr);
    }
}

TEST_CASE("fragment") {
    std::array<uint8_t, 256> store;
    free_list alloc(store);

    std::list<void*> ptrs;
    for (int i = 0; i < 3; ++i) {
        ptrs.push_back(alloc.allocate(8));
    }

    for (int i = 0; i < 128; ++i) {
        auto it = std::next(ptrs.begin(), 1);
        alloc.free(*it);
        ptrs.erase(it);
        auto ptr = alloc.allocate(8);
        REQUIRE_NE(nullptr, ptr);
        ptrs.push_back(ptr);
    }
}
} // namespace
} // namespace tos::memory