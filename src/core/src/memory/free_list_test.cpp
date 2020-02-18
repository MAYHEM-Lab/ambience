#include <doctest.h>
#include <tos/memory/free_list.hpp>

namespace tos::memory {
namespace {
TEST_CASE("free list allocation works") {
    std::array<uint8_t, 128> store;
    free_list alloc(store);

    auto ptr = alloc.allocate(4);
    REQUIRE_NE(nullptr, ptr);

    auto ptr2 = alloc.allocate(4);
    REQUIRE_NE(nullptr, ptr2);
    
    REQUIRE(ptr != ptr2);
}
}
}