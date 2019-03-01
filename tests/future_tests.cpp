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
    tos::promise<int> p;

    tos::future<int> f = p.get_future();

    tos::semaphore s{0};
    tos::launch([&]{
        s.down();
        p.set(42);
    });

    s.up();
    REQUIRE(f.get() == 42);
}