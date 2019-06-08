//
// Created by fatih on 11/21/18.
//

#include "doctest.h"
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

TEST_CASE("expected")
{
    auto res = bar();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("expected fwd")
{
    auto res = fwd();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("unexpected")
{
    auto res = foo();
    REQUIRE(!res);
}

TEST_CASE("move")
{
    auto res = bar();
    auto res2 = std::move(res);
    REQUIRE(res2);
    REQUIRE(force_get(res2) == 3);
    REQUIRE(res);
}

TEST_CASE("with")
{
    REQUIRE(with(foo(), [](int x) { return x; }, [](int x){ return x; }) == 3);
    REQUIRE(with(bar(), [](int x) { return x; }, [](int x){ return x; }) == 3);
}

class move_only
{
public:
    move_only() = default;
    move_only(const move_only&) = delete;
    move_only(move_only&& rhs) {
        rhs.moved = true;
    }

    bool moved = false;
};

tos::expected<move_only, int> test_move_only() { return move_only{}; }

TEST_CASE("move only")
{
    auto res = test_move_only();
    auto res2 = std::move(res);
    REQUIRE(res2);
    REQUIRE(res);
    REQUIRE(!force_get(res2).moved);
    REQUIRE(force_get(res).moved);
}