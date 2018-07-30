//
// Created by fatih on 7/28/18.
//
#include <compat/lwipr_compat.h>
#undef putc
#undef getc
#include <tos/ft.hpp>
#include <drivers/arch/lx106/usart.hpp>
#include <drivers/arch/lx106/timer.hpp>
#include <drivers/arch/lx106/wifi.hpp>
#include <drivers/arch/lx106/tcp.hpp>

#include <lwip/init.h>
#include <tos/print.hpp>
#include <drivers/common/inet/tcp_stream.hpp>

char buf[4096];
void main_task()
{
    using namespace tos::tos_literals;
    lwip_init();
    axl_init(10);

    auto uart_params = tos::usart_config()
                        .add(115200_baud_rate)
                        .add(tos::usart_stop_bit::one)
                        .add(tos::usart_parity::disabled);

    auto usart = tos::open(tos::devs::usart<0>, uart_params);

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::esp82::wifi w;

    auto res = w.connect("Nakedsense.2", "serdar1988");
    while(!res)
    {
        res = w.connect("Nakedsense.2", "serdar1988");
    }

    tos::println(usart, "connected");

    with(tos::std::move(res), [&](tos::esp82::wifi_connection& c){
        while (!w.wait_for_dhcp());

        with(tos::esp82::connect(c, { 192, 168, 0, 40 }, { 80 }),
            [&](tos::esp82::tcp_endpoint& ep)
            {
                tos::tcp_stream<tos::esp82::tcp_endpoint> stream(tos::std::move(ep));
                tos::println(stream, "GET /");
                tos::println(stream);

                bool stop = false;
                while (!stop)
                {
                    with(stream.read(buf), [&](auto& res){
                        tos::print(usart, res);
                    }, [&](auto&) {
                        stop = true;
                    });
                }
                tos::println(usart);
            },
        tos::ignore);
    }, tos::ignore);

    while (true)
    {
        alarm.sleep_for({500});

        tos::println(usart, "hello world!");
    }
}

void tos_main()
{
    tos::launch(main_task);
}