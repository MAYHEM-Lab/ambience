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
#include <assert.h>

double GetTemp(void)
{
    assert(true);
    ADMUX = (3 << REFS0) | (8 << MUX0); // 1.1V REF, channel#8 is temperature
    ADCSRA |= (1 << ADEN) | (6 << ADPS0);       // enable the ADC div64
    _delay_ms(20);                  // wait for voltages to become stable.
    ADCSRA |= (1 << ADSC);      // Start the ADC

    while (ADCSRA & (1 << ADSC));       // Detect end-of-conversion
    // The offset of 324.31 could be wrong. It is just an indication.
    // The returned temperature is in degrees Celcius.
    return (ADCW - 324.31) / 1.22;
}

void tick_task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();
    auto comm = open(tos::devs::tty<0>, usart);

    tos::dht d{};

    tos::avr::gpio g;
    g.set_pin_mode(8_pin, tos::pin_mode_t::in_pullup);

    g.set_pin_mode(2_pin, tos::pin_mode_t::in_pullup);

    tos::event temp_int;
    auto inthandler = [&]{
        temp_int.fire_isr();
    };

    g.attach_interrupt(2_pin, tos::pin_change::low, inthandler);

    auto tmr = open(tos::devs::timer<1>);
    tos::alarm<tos::remove_reference_t<decltype(*tmr)>> alarm{*tmr};
    while (true)
    {
        temp_int.wait();
        char b[32];
        auto res = d.read11(8_pin);
        tos::println(*usart, res);
        tos::println(*comm, "Temperature:", dtostrf(d.temperature, 2, 2, b));
        tos::println(*comm, "Humidity:", dtostrf(d.humidity, 2, 2, b));
        auto st = GetTemp();
        tos::println(*comm, "Internal:", dtostrf(st, 2, 2, b));
        alarm.sleep_for({ 100 });
    }
}

int main()
{
    using namespace tos::tos_literals;

    tos::enable_interrupts();

    tos::launch(tick_task);

    while(true)
    {
        tos::schedule();
    }
}