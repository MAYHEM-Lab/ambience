//
// Created by fatih on 12/14/18.
//

#include "doctest.h"
#include <tos/function_ref.hpp>

TEST_CASE("simple")
{
    tos::function_ref<bool()> fr {[](void*){ return true; }};
    REQUIRE(fr());

    fr = tos::function_ref<bool()>([](void*) { return false; });
    REQUIRE(!fr());

    bool b = false;
    auto f = [&]{
        return b;
    };

    fr = tos::function_ref<bool()>(f);

    REQUIRE(!fr());

    b = true;

    REQUIRE(fr());
}