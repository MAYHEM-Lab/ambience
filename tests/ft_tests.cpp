//
// Created by fatih on 11/11/18.
//

#include "doctest.h"
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/suspended_launch.hpp>

namespace tos {
namespace {
TEST_CASE("launch & semaphore task") {
    tos::semaphore s{0};
    tos::launch(
        tos::alloc_stack,
        [](void* sp) {
            static_cast<tos::semaphore*>(sp)->up();
            REQUIRE(true);
        },
        &s);
    s.down();
    REQUIRE(true);
}

TEST_CASE("lambda launch & semaphore task") {
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&] {
        s.up();
        REQUIRE(true);
    });
    s.down();
    REQUIRE(true);
}

TEST_CASE("lambda") {
    int x{};
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&] {
        x = 10;
        s.up();
    });
    s.down();
    REQUIRE(x == 10);
}

TEST_CASE("lambda with args") {
    int x{};
    tos::semaphore s{0};
    tos::launch(
        tos::alloc_stack,
        [&](int a, int b) {
            x = a + b;
            s.up();
        },
        42,
        42);
    s.down();
    REQUIRE(x == 84);
}

TEST_CASE("New threads are runnable by default") {
    semaphore wait{0};

    auto& t = launch(alloc_stack, [&] { wait.up(); });

    this_thread::yield();

    REQUIRE_EQ(1, get_count(wait));
    wait.down();
}

TEST_CASE("Suspended launch works") {
    semaphore wait{0};
    bool explicitly_scheduled = false;
    auto& t = suspended_launch(alloc_stack, [&] {
        REQUIRE(explicitly_scheduled); // should _not_ be reached!
        wait.up();
    });

    this_thread::yield(); // If the new thread is scheduled, the test case should fail.

    explicitly_scheduled = true;
    kern::make_runnable(t);

    wait.down();
}
} // namespace
} // namespace tos