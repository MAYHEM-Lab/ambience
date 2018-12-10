//
// Created by fatih on 12/10/18.
//

#include <tos/ft.hpp>
#include "catch.hpp"
#include <drivers/arch/x86/drivers.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/semaphore.hpp>

TEST_CASE("alarm", "[basic]"){
    tos::semaphore s{0};
    tos::launch([](void* ptr){
        auto& s = *static_cast<tos::semaphore*>(ptr);
        tos::x86::timer tmr;
        auto alarm = tos::open(tos::devs::alarm, tmr);

        for (int i = 0; i < 50; ++i)
        {
            auto tm = std::chrono::system_clock::now();
            using namespace std::chrono_literals;
            alarm.sleep_for(100ms);

            auto diff = std::chrono::system_clock::now() - tm;

            REQUIRE(diff >= 100ms);
            REQUIRE(diff <= 120ms);
        }

        s.up();
    }, &s);
    s.down();
    REQUIRE(true);
}