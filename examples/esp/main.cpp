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

tos::semaphore wifi_sem{0};

void task()
{
    using namespace tos::tos_literals;

    usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    int x = 15;
    int y = 17;

    wifi_set_opmode_current(STATION_MODE);
    wifi_station_scan(nullptr, [](void* arg, STATUS s){
        if (s == OK)
        {
            auto bssInfo = (bss_info *)arg;
            // skip the first in the chain ... it is invalid
            bssInfo = STAILQ_NEXT(bssInfo, next);
            while(bssInfo) {
                //tos::println(*usart, "ssid:", (const char*)bssInfo->ssid);
                bssInfo = STAILQ_NEXT(bssInfo, next);
            }
        }
        wifi_sem.up();
        system_os_post(tos::esp82::main_task_prio, 0, 0);
    });

    wifi_sem.down();

    while (true)
    {
        ++x;

        tos::println(*usart, "Task: On");
        {
            tos::lock_guard<tos::mutex> lk{prot};
            tos::println(*usart, "base sp:", read_sp(), &x, x);
            tos::println(*usart, "base sp:", read_sp(), &y, y);
        }
        sem.up();
        alarm.sleep_for({ 500 });

        sem.up();
        tos::println(*usart, "Task: Off");
        alarm.sleep_for({ 500 });
    }
}

void tos_main()
{
    tos::launch(task);
    tos::launch(other);
}