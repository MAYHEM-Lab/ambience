//
// Created by fatih on 2/28/19.
//

#include "catch.hpp"
#include <tos/future.hpp>

TEST_CASE("future", "[future]")
{
    tos::promise<int> p;

    auto f = p.get_future();

    p.set(42);

    REQUIRE(f.get() == 42);
}

TEST_CASE("future across threads", "[future]")
{
    tos::future<int> f;

    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        tos::promise<int> p;
        f = p.get_future();
        s.up();
        p.set(42);
    });

    s.down();

    REQUIRE(f.get() == 42);
}


TEST_CASE("future across threads, moving the future", "[future]")
{
    tos::future<int> f;

    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        tos::promise<int> p;
        f = p.get_future();
        s.up();
        p.set(42);
    });

    s.down();

    auto f2 = std::move(f);

    REQUIRE(f2.get() == 42);
}