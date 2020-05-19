#include <doctest.h>
#include <tos/meta/offsetof.hpp>

namespace tos::meta {
namespace {
struct s {
    int a;
    float b;
};

TEST_CASE("Offsetof works") {
    REQUIRE_EQ(0, offset_of<&s::a>());
    REQUIRE_EQ(sizeof(int), offset_of<&s::b>());
}
} // namespace
} // namespace tos::meta