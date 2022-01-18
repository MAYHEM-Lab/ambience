//
// Created by fatih on 11/21/18.
//

#include "doctest.h"

#include <tos/compiler.hpp>
#include <tos/expected.hpp>
#include <tos/result.hpp>

namespace tos {
namespace {
NO_INLINE
expected<int, int> foo() {
    return tos::unexpected(3);
}

NO_INLINE
expected<int, int> bar() {
    return 3;
}

NO_INLINE
auto fwd() {
    return bar();
}

TEST_CASE("expected") {
    auto res = bar();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("expected fwd") {
    auto res = fwd();
    REQUIRE(res);
    REQUIRE(force_get(res) == 3);
}

TEST_CASE("unexpected") {
    auto res = foo();
    REQUIRE(!res);
}

TEST_CASE("move") {
    auto res = bar();
    auto res2 = std::move(res);
    REQUIRE(res2);
    REQUIRE(force_get(res2) == 3);
    REQUIRE(res);
}

TEST_CASE("with") {
    REQUIRE(with(
                foo(), [](int x) { return x; }, [](int x) { return x; }) == 3);
    REQUIRE(with(
                bar(), [](int x) { return x; }, [](int x) { return x; }) == 3);
}

class move_only {
public:
    move_only() = default;
    move_only(const move_only&) = delete;
    move_only(move_only&& rhs) {
        rhs.moved = true;
    }

    bool moved = false;
};

expected<move_only, int> test_move_only() {
    return move_only{};
}

TEST_CASE("move only") {
    auto res = test_move_only();
    auto res2 = std::move(res);
    REQUIRE(res2);
    REQUIRE(res);
    REQUIRE(!force_get(res2).moved);
    REQUIRE(force_get(res).moved);
}

TEST_CASE("expected try works") {
    auto failing_op = []() -> expected<int, int> { return unexpected(42); };
    auto wrapping_op = [&]() -> expected<void, int> {
        auto x = EXPECTED_TRY(failing_op());
        // Will not reach here!
        REQUIRE(false);
        return {};
    };
    REQUIRE_EQ(false, bool(wrapping_op()));
}

expected<int, const_string_error> string_err_fn() {
    return const_string_error("nope");
}

result<int> convert() {
    return string_err_fn();
}

any_error just_err() {
    return const_string_error("nope");
}

any_error fwd_err() {
    auto err = just_err();
    return err;
}

TEST_CASE("result works") {
    auto res = convert();
    REQUIRE_FALSE(res);
}

TEST_CASE("any_error works") {
    auto err = fwd_err();
    REQUIRE_EQ("nope", err.message());
}
} // namespace
} // namespace tos
