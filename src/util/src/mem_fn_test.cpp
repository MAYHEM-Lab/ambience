#include <doctest.h>
#include <tos/function_ref.hpp>
#include <tos/functional.hpp>

namespace tos {
namespace {
struct bar {
    int identity() const {
        return x;
    }

    int multiply(int m) const {
        return x * m;
    }

    int x;
};

struct abstract {
    virtual int func(int) = 0;
};

struct concrete : abstract {
    int func(int i) override {
        return i;
    }
};

TEST_CASE("mem_fn works") {
    constexpr auto ident = mem_fn<&bar::identity>();
    constexpr auto mul = mem_fn<&bar::multiply>();

    REQUIRE_EQ(42, ident(bar{42}));
    REQUIRE_EQ(100, ident(bar{100}));

    REQUIRE_EQ(84, mul(bar{42}, 2));
}

TEST_CASE("mem_function_ref works") {
    bar b{42};
    auto ident = mem_function_ref<&bar::identity>(b);
    REQUIRE_EQ(42, ident());
    b = bar{50};
    REQUIRE_EQ(50, ident());
}

TEST_CASE("mem_function_ref works with virtual calls") {
    concrete b;
    auto funref = mem_function_ref<&abstract::func>(b);
    REQUIRE_EQ(42, funref(42));
}
} // namespace
} // namespace tos