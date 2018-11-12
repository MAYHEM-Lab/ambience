//
// Created by fatih on 11/11/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include "catch.hpp"

TEST_CASE("launch & semaphore task", "[ft]")
{
    tos::semaphore s{0};
    tos::launch([](void* sp){
        static_cast<tos::semaphore *>(sp)->up();
        REQUIRE(true);
    }, &s);
    s.down();
    REQUIRE(true);
}
