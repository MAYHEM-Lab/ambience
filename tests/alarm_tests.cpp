//
// Created by fatih on 12/10/18.
//

#include "doctest.h"

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

TEST_CASE("alarm") {
    tos::semaphore s{0};
    tos::launch(tos::alloc_stack, [&] {
        tos::x86::timer tmr;
        tos::alarm alarm(&tmr);

        for (int i = 0; i < 50; ++i) {
            auto tm = std::chrono::system_clock::now();
            using namespace std::chrono_literals;
            tos::this_thread::sleep_for(alarm, 100ms);
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - tm);

            CHECK_LE(100, diff.count());
            CHECK_GE(130, diff.count());
        }

        s.up();
    });
    s.down();
    REQUIRE(true);
}