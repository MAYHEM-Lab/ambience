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
#include <tos/waitable.hpp>
#include <stdlib.h>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>

tos::usart comm;

int32_t x_ticks = 0;
uint16_t ticks = 0;

double GetTemp(void)
{
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
    auto tmr = open(tos::devs::timer<1>);
    //tmr->disable();
    tmr->set_frequency(1000);
    tmr->enable();
    tmr->set_callback([](void* d)
    {
        x_ticks++;
    }, nullptr);

    tos::dht d{};
    auto p = 8_pin;
    tos::avr::gpio g;
    g.set_pin_mode(p, tos::pin_mode_t::in_pullup);

    while (true)
    {
        tmr->block();
        ticks++;
        if (ticks == 2000)
        {
            char b[32];
            println(comm, "Tick!", int(d.read11(8_pin)), dtostrf(d.temperature, 2, 2, b));
            println(comm, "Humidity: ", dtostrf(d.humidity, 2, 2, b));
            auto st = GetTemp();
            println(comm, "Internal: ", dtostrf(st, 2, 2, b));
            ticks = 0;
        }
    }
}

int main()
{
    tos::enable_interrupts();

    tos::launch(tick_task);

    while(true)
    {
        tos::schedule();
    }
}