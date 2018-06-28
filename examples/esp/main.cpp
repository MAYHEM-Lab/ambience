//
// Created by fatih on 4/26/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/fixed_fifo.hpp>

#include <drivers/arch/lx106/timer.hpp>
#include <drivers/arch/lx106/usart.hpp>
#include <drivers/arch/lx106/wifi.hpp>
#include <drivers/arch/lx106/tcp.hpp>

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

tos::fixed_fifo<int, 24> buf;
void task()
{
    using namespace tos::tos_literals;

    usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    tos::print(*usart, "\n\n\n\n\n\n");

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = open(tos::devs::alarm, tmr);

    tos::esp82::wifi w;
    auto res = w.connect("WIFI", "PASS");
    while (!w.wait_for_dhcp());

    tos::println(*usart, "connected?", res);

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(*usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 1000 } };

    auto handler = [&sock](espconn* new_conn){
        buf.push(42);
        buf.push((int)new_conn);
        buf.push((int)new_conn->reverse);
        new_conn->reverse = (void*)2345;

        espconn_regist_recvcb(new_conn, [](void* arg, char *pdata, unsigned short len){
            auto new_conn = (espconn *)arg;
            buf.push(len);
            buf.push((int)arg);
            buf.push((int)new_conn->reverse);
            system_os_post(tos::esp82::main_task_prio, 0, 0);

            espconn_send(new_conn, (uint8_t *)"hello", 5);
        });

        system_os_post(tos::esp82::main_task_prio, 0, 0);
    };

    sock.accept(handler);

    buf.push(100);

    while (true)
    {
        auto c = buf.pop();
        tos::println(*usart, c);
    }

    /*while (true)
    {
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
    }*/
}

void tos_main()
{
    tos::launch(task);
}