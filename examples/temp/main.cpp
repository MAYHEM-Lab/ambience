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

template <class AlarmT>
double GetTemp(AlarmT&& alarm)
{
    ADMUX = (3 << REFS0) | (8 << MUX0); // 1.1V REF, channel#8 is temperature
    ADCSRA |= (1 << ADEN) | (6 << ADPS0);       // enable the ADC div64
    alarm.sleep_for({20});
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
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);


    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(8_pin, tos::pin_mode::in_pullup);

    g.set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event temp_int;
    auto inthandler = [&]{
        temp_int.fire_isr();
    };

    g.attach_interrupt(2_pin, tos::pin_change::low, inthandler);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    auto d = tos::make_dht(g, [](tos::microseconds us) {
        _delay_us(us.val);
    });
    while (true)
    {
        temp_int.wait();
        char b[32];
        auto res = d.read11(8_pin);
        tos::println(usart, int8_t(res));
        tos::println(usart, "Temperature:", dtostrf(d.temperature, 2, 2, b));
        tos::println(usart, "Humidity:", dtostrf(d.humidity, 2, 2, b));
        auto st = GetTemp(alarm);
        tos::println(usart, "Internal:", dtostrf(st, 2, 2, b));
        alarm.sleep_for({ 100 });
    }
}

void tos_main()
{
    tos::launch(main_task);
}