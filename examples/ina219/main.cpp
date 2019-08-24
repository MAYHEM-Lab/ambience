//
// Created by fatih on 11/8/18.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <common/ina219.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

void main_task() {
    using namespace tos;
    using namespace tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::println(usart, "hello");

    auto i2c = tos::open(tos::devs::i2c<0>, tos::i2c_type::master, 19_pin, 18_pin);
    ina219<decltype(&i2c)> ina{twi_addr_t{0x41}, &i2c};

    auto tmr = open(devs::timer<1>);
    auto alarm = open(devs::alarm, tmr);

    while (true) {
        using namespace std::chrono_literals;
        alarm.sleep_for(500ms);
        int curr = ina.getCurrent_mA();
        int v = ina.getBusVoltage_V();

        tos::println(usart, "V:", v);
        tos::println(usart, "I:", curr, "mA");
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_task);
}