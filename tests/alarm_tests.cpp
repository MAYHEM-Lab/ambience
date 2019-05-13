//
// Created by fatih on 12/10/18.
//

#include <tos/ft.hpp>
#include "catch.hpp"
#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/semaphore.hpp>

TEST_CASE("alarm", "[basic]"){
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&]{
        tos::x86::timer tmr;
        auto alarm = tos::open(tos::devs::alarm, tmr);

        for (int i = 0; i < 50; ++i)
        {
            auto tm = std::chrono::system_clock::now();
            using namespace std::chrono_literals;
            alarm.sleep_for(100ms);

            auto diff = std::chrono::system_clock::now() - tm;

            REQUIRE(diff >= 100ms);
            REQUIRE(diff <= 130ms);
        }

        s.up();
    });
    s.down();
    REQUIRE(true);
}