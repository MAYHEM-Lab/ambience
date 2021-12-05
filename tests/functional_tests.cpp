#include <doctest.h>
#include <tos/functional.hpp>

namespace tos {
namespace {
TEST_CASE("tos::mem_fn works") {
    struct x {
        int y = 42;
        const int& foo() const {
            return y;
        }
    };
    auto fn = mem_fn<&x::foo>();
    REQUIRE_EQ(42, fn(x{}));
}
} // namespace
} // namespace tos