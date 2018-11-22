//
// Created by Mehmet Fatih BAKIR on 11/10/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <drivers/common/xbee.hpp>
#include <drivers/common/alarm.hpp>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>
#include <tos/semaphore.hpp>

#include <tos/mem_stream.hpp>

void tx_task(void*)
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto gpio = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<1>);
    auto alarm = tos::open(tos::devs::alarm, *tmr);

    tos::println(usart, "send a character to get into the debugger");
    char debug_buf[1];
    auto res = usart.read(debug_buf, alarm, 5s);

    if (res.size() != 0)
    {
        tos::println(usart, "hello! you're in the debugger");

        bool debug = true;

        while (debug)
        {
            auto res = usart.read(debug_buf); // wait for a command, possible forever

            switch (res[0])
            {
                case 'c':
                    debug = false;
                    break;
                default: break;
            }
        }

        tos::println(usart, "continuing...");
    }

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(13_pin, tos::pin_mode::out);
    gpio.write(7_pin, tos::digital::low);

    int seq = 0;

    while (true)
    {
        namespace xbee = tos::xbee;
        usart->clear();

        gpio.write(7_pin, tos::digital::high);
        alarm.sleep_for(100ms);
        auto r = xbee::read_modem_status(usart, alarm);

        xbee::tx_status stat;
        stat.status = xbee::tx_status::statuses::cca_fail;
        if (r)
        {
            tos::xbee_s1<tos::avr::usart0> x {usart};

            static char msg_buf[100];
            tos::omemory_stream buff{msg_buf};

            tos::println(buff, seq++);

            constexpr xbee::addr_16 base_addr{ 0x0010 };
            xbee::tx16_req req { base_addr, tos::raw_cast<const uint8_t>(buff.get()), xbee::frame_id_t{1} };
            x.transmit(req);

            alarm.sleep_for(100ms);
            int retries = 5;
            while (stat.status != xbee::tx_status::statuses::success && retries --> 0)
            {
                auto tx_r = xbee::read_tx_status(usart, alarm);
                if (tx_r)
                {
                    stat = force_get(tx_r);
                }
                else
                {
                    stat.status = xbee::tx_status::statuses::cca_fail;
                }
            }
        }

        gpio.write(7_pin, tos::digital::low);

        tos::println(usart, int(stat.status));

        gpio.write(13_pin, tos::digital::high);
        alarm.sleep_for(1s);
        gpio.write(13_pin, tos::digital::low);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}