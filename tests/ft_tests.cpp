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

TEST_CASE("lambda launch & semaphore task", "[ft]")
{
    tos::semaphore s{0};
    tos::launch(+[](void* sp){
        static_cast<tos::semaphore *>(sp)->up();
        REQUIRE(true);
    }, &s);
    s.down();
    REQUIRE(true);
}

TEST_CASE("lambda", "[ft]")
{
    int x{};
    tos::semaphore s{0};
    tos::launch([&]{
        x = 10;
        s.up();
    });
    s.down();
    REQUIRE(x == 10);
}

TEST_CASE("lambda with args", "[ft]")
{
    int x{};
    tos::semaphore s{0};
    tos::launch([&](int a, int b){
        x = a + b;
        s.up();
    }, 42, 42);
    s.down();
    REQUIRE(x == 84);
}