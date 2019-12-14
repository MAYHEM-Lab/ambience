//
// Created by fatih on 12/10/18.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <iostream>
#include <tos/ft.hpp>

void tos_main() {
    tos::launch(tos::alloc_stack, [] {
        auto timer = tos::open(tos::devs::timer<0>);
        auto alarm = tos::open(tos::devs::alarm, timer);

        while (true) {
            std::cout << "hi" << '\n';
            using namespace std::chrono_literals;
            tos::this_thread::sleep_for(alarm, 1s);
        }
    });
}