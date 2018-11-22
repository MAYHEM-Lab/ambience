//
// Created by fatih on 11/21/18.
//

#include "catch.hpp"
#include <tos/expected.hpp>
#include <tos/compiler.hpp>

tos::expected<int, int> NO_INLINE foo()
{
    return tos::unexpected(3);
}

tos::expected<int, int> NO_INLINE bar()
{
    return 3;
}

auto NO_INLINE fwd()
{
    return bar();
}

TEST_CASE("expected", "[expected]")
{
    auto res = bar();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("expected fwd", "[expected]")
{
    auto res = fwd();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("unexpected", "[expected]")
{
    auto res = foo();
    REQUIRE(!res);
}

TEST_CASE("move", "[expected]")
{
    auto res = bar();
    auto res2 = std::move(res);
    REQUIRE(res2);
    REQUIRE(force_get(res2) == 3);
    REQUIRE(res);
}
