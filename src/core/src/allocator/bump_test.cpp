#include <doctest.h>
#include <tos/memory/bump.hpp>

namespace tos::memory {
namespace {
TEST_CASE("bump allocator works") {
    std::vector<uint8_t> base_mem(4096);
    bump_allocator alloc(base_mem);

    auto ptr = alloc.allocate(1024);

    REQUIRE_NE(0xFF, *static_cast<uint8_t*>(ptr));
    // Fill the buffer with 0xFFs so we can see the memory is coming from the buffer
    // we created.
    std::fill(base_mem.begin(), base_mem.end(), 0xFF);
    REQUIRE_EQ(0xFF, *static_cast<uint8_t*>(ptr));
}
}
}