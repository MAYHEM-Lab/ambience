//
// Created by fatih on 12/23/18.
//

#include <tos/ft.hpp>
#include <common/adxl345.hpp>
#include <arch/lx106/drivers.hpp>
#include <tos/print.hpp>
#include <common/gpio.hpp>

tos::stack_storage<1024> accel_stack;
auto task = [] {
    using namespace tos::tos_literals;
    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::println(usart, "hello world!");

    tos::esp82::twim twim{ {5}, {4} };

    tos::adxl345 sensor{twim};
    sensor.powerOn();
    sensor.setRangeSetting(2);

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    while (true)
    {
        auto [x, y, z] = sensor.read();

        tos::println(usart, x, y, z);

        using namespace std::chrono_literals;
        alarm->sleep_for(200ms);
    }
};

auto wifi_connect()
{
    tos::esp82::wifi w;

    conn_:
    auto res = w.connect("mayhem", "z00mz00m");
    if (!res) goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();
    lwip_init();

    return std::make_pair(w, std::move(wconn));
}

auto wifi_task = []{
    auto [w, conn] = wifi_connect();


};

void tos_main()
{
    tos::launch(accel_stack, task);
    tos::launch(tos::alloc_stack, wifi_task);
}
