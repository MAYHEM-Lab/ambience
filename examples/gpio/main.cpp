//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

auto gp = open(tos::devs::gpio);

using namespace tos;
using namespace tos::tos_literals;

auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

void hello_task() {
    using namespace tos::tos_literals;
    gp->set_pin_mode(13_pin, tos::pin_mode::out);
    gp->write(13_pin, false);
    char buf;
    while (true) {
        usart->read({&buf, 1});
        if (buf == '1') {
            gp->write(13_pin, true);
            tos::println(*usart, "On");
        } else {
            gp->write(13_pin, false);
            tos::println(*usart, "Off");
        }
    }
}

void tick_task() {
    using namespace tos::tos_literals;

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    while (true) {
        using namespace std::chrono_literals;
        alarm.sleep_for(1s);
        tos::println(*usart, "Tick");
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, hello_task);
    tos::launch(tos::alloc_stack, tick_task);
}