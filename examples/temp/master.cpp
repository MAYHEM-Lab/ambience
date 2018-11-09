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
#include <util/include/tos/mem_stream.hpp>
#include "app.hpp"
#include <drivers/common/ina219.hpp>

namespace temp
{
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
            return temp::sample{ -5, -6, -7 };
        }

        std::array<char, sizeof(temp::sample)> buf;

        tos::print(usart, "go");
        usart.read(buf);

        std::array<char, 1> chk_buf;
        auto chkbuf = usart.read(chk_buf);

        gpio.write(11_pin, tos::digital::low);
        alarm.sleep_for(5ms);

        uint8_t chk = 0;
        for (char c : buf)
        {
            chk += uint8_t (c);
        }

        usart.write(buf);
        usart.write(chkbuf);

        usart.write(tos::raw_cast<char>(tos::span<uint8_t>{&chk,1}));

        if (uint8_t(chkbuf[0]) != chk)
        {
            return temp::sample{};
        }

        temp::sample s;
        memcpy(&s, buf.data(), sizeof(temp::sample));
        return s;
    }
}

/**
 * This function takes the system to a deep sleep.
 *
 * The actual sleep duration could be longer than the requested time.
 *
 * @param dur seconds to at least sleep for
 */
void hibernate(std::chrono::seconds dur)
{
    using namespace std::chrono_literals;

    while (dur > 8s)
    {
        wdt_enable(WDTO_8S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();
        dur -= 8s;
    }

    while (dur > 4s)
    {
        wdt_enable(WDTO_4S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();
        dur -= 4s;
    }

    while (dur > 2s)
    {
        wdt_enable(WDTO_2S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();
        dur -= 2s;
    }

    while (dur > 1s)
    {
        wdt_enable(WDTO_1S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();
        dur -= 1s;
    }
}

void tx_task(void*)
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto gpio = tos::open(tos::devs::gpio);

    avr::twim t{18_pin, 19_pin};

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(11_pin, tos::pin_mode::out);
    gpio.set_pin_mode(12_pin, tos::pin_mode::in_pullup);
    gpio.write(7_pin, tos::digital::low);
    gpio.write(11_pin, tos::digital::low);

    while (true)
    {
        using namespace std::chrono_literals;
        {
            auto tmr = tos::open(tos::devs::timer<1>);
            auto alarm = tos::open(tos::devs::alarm, *tmr);

            ina219<avr::twim> ina{ {0x41}, t };

            using namespace std::chrono_literals;
            alarm.sleep_for(2s);

            gpio.set_pin_mode(12_pin, tos::pin_mode::in_pullup);

            auto d = tos::make_dht(gpio, [](std::chrono::microseconds us) {
                _delay_us(us.count());
            });

            auto res = d.read(12_pin);

            while (res != tos::dht_res::ok)
            {
                alarm.sleep_for(2s);
                res = d.read(12_pin);
            }

            std::array<temp::sample, 2> samples = {
                    temp::sample{ d.temperature, d.humidity, temp::GetTemp(alarm) },
                    temp::read_slave(gpio, alarm, usart)
            };

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

            static char msg_buf[100];
            tos::memory_stream buff{msg_buf};

            static char fl_buf[10];
            tos::print(buff, temp::master_id, "");
            tos::print(buff, 1, dtostrf(samples[0].temp, 2, 2, fl_buf), "");
            tos::print(buff, 2, dtostrf(samples[0].humid, 2, 2, fl_buf), "");
            tos::print(buff, 3, dtostrf(samples[0].cpu, 2, 2, fl_buf));
            tos::println(buff);

            tos::print(buff, temp::slave_id, "");
            tos::print(buff, 1, dtostrf(samples[1].temp, 2, 2, fl_buf), "");
            tos::print(buff, 2, dtostrf(samples[1].humid, 2, 2, fl_buf), "");
            tos::print(buff, 3, dtostrf(samples[1].cpu, 2, 2, fl_buf));
            tos::println(buff);

            constexpr xbee::addr_16 base_addr { 0x0010};

            xbee::tx16_req r { base_addr, tos::raw_cast<const uint8_t>(buff.get()), fid };

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
            alarm.sleep_for(5ms);
        }

        hibernate(4min + 50s);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}