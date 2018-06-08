//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/gpio.hpp>
#include <tos/devices.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <stdlib.h>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>
#include <tos/event.hpp>
#include <drivers/common/alarm.hpp>
#include <avr/io.h>
#include <tos/compiler.hpp>

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

void main_task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    tos::dht d{};

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(8_pin, tos::pin_mode::in_pullup);

    g->set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event temp_int;
    auto inthandler = [&]{
        temp_int.fire_isr();
    };

    g->attach_interrupt(2_pin, tos::pin_change::low, inthandler);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    while (true)
    {
        temp_int.wait();
        char b[32];
        auto res = d.read11(8_pin);
        tos::println(*usart, res);
        tos::println(*usart, "Temperature:", dtostrf(d.temperature, 2, 2, b));
        tos::println(*usart, "Humidity:", dtostrf(d.humidity, 2, 2, b));
        auto st = GetTemp(alarm);
        tos::println(*usart, "Internal:", dtostrf(st, 2, 2, b));
        alarm.sleep_for({ 100 });
    }
}

void tos_main()
{
    tos::launch(main_task);
}