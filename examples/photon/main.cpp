//
// Created by fatih on 10/25/18.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

void blink_task() {
    using namespace tos::tos_literals;

    auto g = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<3>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    g.set_pin_mode(1_pin, tos::pin_mode::out);

    while (true) {
        using namespace std::chrono_literals;
        g.write(1_pin, tos::digital::high);
        alarm.sleep_for(1s);

        g.write(1_pin, tos::digital::low);
        alarm.sleep_for(1s);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, blink_task);
}
