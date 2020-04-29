//
// Created by fatih on 12/14/18.
//

#include "doctest.h"

#include <tos/function_ref.hpp>

namespace tos {
namespace {
TEST_CASE("simple") {
    tos::function_ref<bool()> fr{[](void*) { return true; }};
    REQUIRE(fr());

    fr = tos::function_ref<bool()>([](void*) { return false; });
    REQUIRE(!fr());

    bool b = false;
    auto f = [&] { return b; };

    fr = tos::function_ref<bool()>(f);

    REQUIRE(!fr());

    b = true;

    REQUIRE(fr());
}

TEST_CASE("unsafe function cast works") {
    bool b = false;
    auto f = [&] { return b; };

    auto fr = tos::function_ref<bool()>(f);

    REQUIRE_EQ(false, fr());

    auto unsafe = unsafe_function_ref_cast<function_ref<void()>>(fr);

    auto safe = unsafe_function_ref_cast<function_ref<bool()>>(unsafe);

    REQUIRE_EQ(false, safe());
}
} // namespace
} // namespace tos