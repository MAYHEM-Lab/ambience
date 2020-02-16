//
// Created by fatih on 12/10/18.
//

#include <tos/ft.hpp>
#include "doctest.h"
#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/semaphore.hpp>

TEST_CASE("alarm"){
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        tos::x86::timer tmr;
        auto alarm = tos::open(tos::devs::alarm, tmr);

        for (int i = 0; i < 50; ++i)
        {
            auto tm = std::chrono::system_clock::now();
            using namespace std::chrono_literals;
            tos::this_thread::sleep_for(alarm, 100ms);
            auto diff = std::chrono::system_clock::now() - tm;

            REQUIRE_LE(100ms, diff);
            REQUIRE_GE(130ms, diff);
        }

        s.up();
    });
    s.down();
    REQUIRE(true);
}