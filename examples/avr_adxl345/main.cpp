//
// Created by fatih on 12/23/18.
//

#include <arch/drivers.hpp>
#include <common/adxl345.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>

auto task = [] {
    using namespace tos::tos_literals;
    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(7_pin, tos::pin_mode::out);
    g.write(7_pin, tos::digital::high);

    tos::println(usart, "hello", int32_t(F_CPU));

    auto i2c = tos::open(tos::devs::i2c<0>, tos::i2c_type::master, 19_pin, 18_pin);

    tos::adxl345<decltype(&i2c)> sensor{&i2c};
    sensor.powerOn();
    tos::println(usart, "on");
    sensor.setRangeSetting(2);
    tos::println(usart, "on");

    auto res = sensor.read();

    tos::println(usart, res.x, res.y, res.z);
    tos::this_thread::block_forever();
};

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
