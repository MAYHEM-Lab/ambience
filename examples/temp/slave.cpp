//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/gpio.hpp>
#include <tos/devices.hpp>
#include <stdlib.h>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>
#include <tos/event.hpp>
#include <drivers/common/alarm.hpp>
#include <avr/io.h>
#include <tos/compiler.hpp>
#include <drivers/common/xbee.hpp>
#include <ft/include/tos/semaphore.hpp>
#include "app.hpp"

template <class AlarmT>
double GetTemp(AlarmT&& alarm)
{
    ADMUX = (3 << REFS0) | (8 << MUX0); // 1.1V REF, channel#8 is temperature
    ADCSRA |= (1 << ADEN) | (6 << ADPS0);       // enable the ADC div64

    using namespace std::chrono_literals;
    alarm.sleep_for(20ms);

    ADCSRA |= (1 << ADSC);      // Start the ADC

    while (ADCSRA & (1 << ADSC));       // Detect end-of-conversion

    // The offset of 324.31 could be wrong. It is just an indication.
    // The returned temperature is in degrees Celcius.
    return (ADCW - 324.31) / 1.22;
}

void main_task(void*)
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(10_pin, tos::pin_mode::in_pullup);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    auto d = tos::make_dht(g, [](std::chrono::microseconds us) {
        _delay_us(us.count());
    });

    std::array<char, 2> buf;
    usart.read(buf);

    auto res = d.read(10_pin);

    if (buf[1] == 'o')
    {
        temp::sample s { d.temperature, d.humidity };
        usart.write({ (const char*)&s, sizeof s });
    }
    else
    {
        static char b[32];
        tos::println(usart, int8_t(res));
        tos::println(usart, "Temperature:", dtostrf(d.temperature, 2, 2, b));
        tos::println(usart, "Humidity:", dtostrf(d.humidity, 2, 2, b));
    }

    tos::semaphore{0}.down();
    //auto st = GetTemp(alarm);
    //tos::println(usart, "Internal:", dtostrf(st, 2, 2, b));
}

void tos_main()
{
    tos::launch(main_task);
}