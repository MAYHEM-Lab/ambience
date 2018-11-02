//
// Created by Mehmet Fatih BAKIR on 11/10/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <drivers/common/xbee.hpp>
#include <drivers/common/alarm.hpp>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

#include <array>
#include <avr/wdt.h>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>
#include "app.hpp"

namespace temp
{
    template <class GpioT, class AlarmT>
    tos::expected<sample, tos::dht_res> read_self(GpioT& gpio, AlarmT&)
    {
        using namespace tos::tos_literals;
        gpio.set_pin_mode(10_pin, tos::pin_mode::in_pullup);

        auto dht = tos::make_dht(gpio, [](std::chrono::microseconds us) {
            _delay_us(us.count());
        });

        auto res = dht.read(10_pin);

        if (res != tos::dht_res::ok)
        {
            return tos::unexpected(res);
        }

        return sample{ dht.temperature, dht.humidity };
    }

    template <class GpioT, class AlarmT, class UsartT>
    sample read_slave(GpioT& gpio, AlarmT& alarm, UsartT& usart)
    {
        using namespace tos::tos_literals;
        using namespace std::chrono_literals;

        gpio.write(11_pin, tos::digital::high);

        std::array<char, 2> wait;
        usart.read(wait);

        if (wait[0] != 'h' || wait[1] != 'i')
        {
            while (true)
            {
                gpio.write(13_pin, tos::digital::high);
                _delay_ms(500);

                gpio.write(13_pin, tos::digital::low);
                _delay_ms(500);
            }
        }

        static std::array<char, sizeof(temp::sample)> buf;

        tos::print(usart, "go");
        usart.read(buf);

        auto& data = *reinterpret_cast<temp::sample*>(buf.data());

        gpio.write(11_pin, tos::digital::low);

        return data;
    }
}

void tx_task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);


    auto gpio = tos::open(tos::devs::gpio);

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(11_pin, tos::pin_mode::out);
    gpio.set_pin_mode(13_pin, tos::pin_mode::out);
    gpio.write(7_pin, tos::digital::low);
    gpio.write(11_pin, tos::digital::low);

    while (true)
    {
        using namespace std::chrono_literals;

        {
            auto usart = open(tos::devs::usart<0>, usconf);

            auto tmr = tos::open(tos::devs::timer<1>);
            auto alarm = tos::open(tos::devs::alarm, *tmr);

            std::array<temp::sample, 2> samples;
            //auto my = temp::read_self(gpio, alarm);

            /*samples[0] = with(std::move(my),
                [](auto& r) { return r; },
                [](auto) { return temp::sample{0, 0}; });*/

            samples[0] = {};
            samples[1] = temp::read_slave(gpio, alarm, usart);

            namespace xbee = tos::xbee;

            gpio.write(7_pin, tos::digital::high);
            {
                xbee::sm_response_parser<xbee::modem_status> parser;
                while (!parser.finished())
                {
                    std::array<char, 1> rbuf;
                    usart.read(rbuf);
                    parser.consume(rbuf[0]);
                }
            }

            tos::xbee_s1<tos::avr::usart0> x {usart};
            xbee::frame_id_t fid{1};

            constexpr xbee::addr_16 base_addr { 0xABCD };
            std::array<uint8_t, samples.size() * sizeof samples[0]> buf;
            memcpy(buf.data(), samples.data(), samples.size() * sizeof samples[0]);
            xbee::tx16_req r { base_addr, buf, fid };

            x.transmit(r);
            fid.id++;

            xbee::sm_response_parser<xbee::tx_status> parser;
            while (!parser.finished())
            {
                std::array<char, 1> rbuf;
                usart.read(rbuf);
                parser.consume(rbuf[0]);
            }

            gpio.write(7_pin, tos::digital::low);
        }

        wdt_enable(WDTO_1S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();
    }
}

void tos_main()
{
    tos::launch(tx_task);
}