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

void tx_task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto gpio = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<1>);
    auto alarm = tos::open(tos::devs::alarm, *tmr);

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(11_pin, tos::pin_mode::out);
    gpio.set_pin_mode(13_pin, tos::pin_mode::out);
    gpio.write(7_pin, tos::digital::low);
    gpio.write(11_pin, tos::digital::low);

    while (true)
    {
        gpio.write(13_pin, tos::digital::low);
        using namespace std::chrono_literals;

        namespace xbee = tos::xbee;

        gpio.write(7_pin, tos::digital::high);

        {
            xbee::sm_response_parser<xbee::modem_status> parser;
            while (!parser.finished())
            {
                std::array<char, 1> rbuf;
                usart.read(rbuf);
                gpio.write(13_pin, tos::digital::high);
                parser.consume(rbuf[0]);
            }
        }

        tos::xbee_s1<tos::avr::usart0> x {usart};
        xbee::frame_id_t fid{1};

        constexpr xbee::addr_16 base_addr { 0xABCD };
        std::array<uint8_t, 2> buf = { 'h', 'i' };
        xbee::tx16_req r { base_addr, buf, fid };

        x.transmit(r);
        fid.id++;

        xbee::sm_response_parser<xbee::tx_status> parser;
        while (!parser.finished())
        {
            std::array<char, 1> rbuf;
            usart.read(rbuf);
            gpio.write(13_pin, tos::digital::high);
            parser.consume(rbuf[0]);
        }

        gpio.write(7_pin, tos::digital::low);

        tos::println(usart, "sent", int(parser.get_len().len), int(parser.get_api_id()));

        alarm.sleep_for(5s);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}