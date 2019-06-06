//
// Created by fatih on 11/11/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include "doctest.h"

TEST_CASE("launch & semaphore task")
{
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [](void* sp){
        static_cast<tos::semaphore *>(sp)->up();
        REQUIRE(true);
    }, &s);
    s.down();
    REQUIRE(true);
}

TEST_CASE("lambda launch & semaphore task")
{
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        s.up();
        REQUIRE(true);
    });
    s.down();
    REQUIRE(true);
}

TEST_CASE("lambda")
{
    int x{};
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        x = 10;
        s.up();
    });
    s.down();
    REQUIRE(x == 10);
}

TEST_CASE("lambda with args")
{
    int x{};
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&](int a, int b){
        x = a + b;
        s.up();
    }, 42, 42);
    s.down();
    REQUIRE(x == 84);
}