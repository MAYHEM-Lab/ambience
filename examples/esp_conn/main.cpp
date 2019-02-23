//
// Created by fatih on 7/19/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>

char buf[512];
void task()
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    int state = 0;
    int i = 0;
    tos::semaphore s{0};
    auto log_ip_task = [&, tcb = tos::impl::cur_thread]
    {
        auto timer = tos::open(tos::devs::timer<0>);
        auto alarm = tos::open(tos::devs::alarm, timer);

        tos::println(usart, "Logger thread running");
        tos::println(usart, "Logger thread:", tos::impl::cur_thread->get_context()[0]);

        while (true)
        {
            alarm.sleep_for(1s);

            tos::println(usart, "Main thread:", state, i, tcb->get_context()[0]);
        }
    };

    tos::launch(log_ip_task);

    tos::esp82::wifi w;
    conn_:
    auto res = w.connect("cs190b", "cs190bcs190b");
    state = 1;

    //tos::println(usart, "connected?", bool(res));
    if (!res) goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    auto addr = force_get(wconn.get_addr());
    state = 2;

    //tos::println(usart, "ip:", int(addr.addr[0]), int(addr.addr[1]), int(addr.addr[2]), int(addr.addr[3]));

    lwip_init();

    for (; true; ++i) {
        auto try_conn = tos::esp82::connect(wconn, {{192, 168, 2, 14}}, {8080});
        state = 3;

        if (!try_conn)
        {
            //tos::println(usart, "couldn't connect");
            continue;
        }

        state = 4;

        auto& conn = force_get(try_conn);
        tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};

        state = 5;

        stream.write("GET / HTTP/1.1\r\n"
                     "Host: 192.168.2.14\r\n"
                     "Connection: close\r\n"
                     "\r\n");

        state = 6;

        while (true)
        {
            auto read_res = stream.read(buf);
            if (!read_res) break;

            auto& r = force_get(read_res);
            //tos::print(usart, r);
            tos::this_thread::yield();
        }

        state = 7;

        //alarm.sleep_for(1s);
        //tos::println(usart, "done", i, int(system_get_free_heap_size()));
    }
}

void tos_main()
{
    tos::launch(task);
}
