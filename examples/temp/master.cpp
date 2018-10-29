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

        gpio.write(8_pin, tos::digital::high);

        alarm.sleep_for(2000ms);
        static std::array<char, sizeof(temp::sample)> buf;

        tos::print(usart, "go");
        usart.read(buf);

        auto& data = *reinterpret_cast<temp::sample*>(buf.data());

        gpio.write(8_pin, tos::digital::low);

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

    auto usart = open(tos::devs::usart<0>, usconf);

    auto gpio = tos::open(tos::devs::gpio);

    gpio.set_pin_mode(7_pin, tos::pin_mode::out);
    gpio.set_pin_mode(8_pin, tos::pin_mode::out);
    gpio.write(7_pin, tos::digital::low);
    gpio.write(8_pin, tos::digital::low);

    while (true)
    {
        using namespace std::chrono_literals;

        {
            auto tmr = tos::open(tos::devs::timer<1>);
            auto alarm = tos::open(tos::devs::alarm, *tmr);

            std::array<temp::sample, 2> samples;
            //auto my = temp::read_self(gpio, alarm);

            /*samples[0] = with(std::move(my),
                [](auto& r) { return r; },
                [](auto) { return temp::sample{0, 0}; });*/

            samples[0] = {};
            samples[1] = temp::read_slave(gpio, alarm, usart);

            gpio.write(7_pin, tos::digital::high);

            alarm.sleep_for(50ms);

            namespace xbee = tos::xbee;

            tos::xbee_s1<tos::avr::usart0> x {usart};
            xbee::frame_id_t fid{1};

            constexpr xbee::addr_16 base_addr { 0xABCD };
            std::array<uint8_t, samples.size()> buf;
            memcpy(buf.data(), samples.data(), samples.size());
            xbee::tx16_req r { base_addr, buf, fid };

            x.transmit(r);
            fid.id++;

            alarm.sleep_for(20ms);
            gpio.write(7_pin, tos::digital::low);
        }

        wdt_enable(WDTO_1S);
        WDTCSR |= (1 << WDIE);
        wait_wdt();

        //alarm.sleep_for(1s);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}