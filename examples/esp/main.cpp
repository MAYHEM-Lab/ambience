//
// Created by fatih on 4/26/18.
//

#include <drivers/arch/lx106/usart.hpp>
#include <arch/lx106/tos_arch.hpp>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <drivers/arch/lx106/timer.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <drivers/arch/lx106/wifi.hpp>

static const int pin = 2;

tos::semaphore sem{0};

tos::mutex prot;
tos::lx106::uart0* usart;

void other()
{
    int x = 5;
    while (true)
    {
        sem.down();
        ++x;
        {
            tos::lock_guard<tos::mutex> lk{prot};
            tos::println(*usart, "Other: On,", &x, x);
        }

        sem.down();
        tos::println(*usart, "Other: Off");
    }
}

void task()
{
    using namespace tos::tos_literals;

    usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    tos::print(*usart, "\n\n\n\n\n\n");

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    int x = 15;
    int y = 17;

    tos::esp82::wifi w;
    auto res = w.connect("Otsimo", "987456321");
    while (!w.wait_for_dhcp());

    tos::println(*usart, "connected?", res);

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(*usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    while (true)
    {
        ++x;

        tos::println(*usart, "Task: On");
        {
            tos::lock_guard<tos::mutex> lk{prot};
            tos::println(*usart, "base sp:", read_sp(), &y, y);
        }
        //sem.up();
        alarm.sleep_for({ 500 });

        //sem.up();
        tos::println(*usart, "Task: Off");
        alarm.sleep_for({ 500 });
    }
}

void tos_main()
{
    tos::launch(task);
}