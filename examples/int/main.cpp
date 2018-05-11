//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <tos/devices.hpp>
#include <tos/interrupt.hpp>
#include <tos/semaphore.hpp>
#include <drivers/arch/avr/gpio.hpp>
#include <tos/event.hpp>

#include <tos/tuple.hpp>
#include <drivers/arch/avr/eeprom.hpp>

tos::usart comm;

tos::event pinsem;

void tick_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(2_pin, tos::pin_mode_t::in_pullup);

    g->attach_interrupt(2_pin, tos::pin_change::falling, {nullptr, [](void*){
        pinsem.fire();
    }});

    tos::tuple<int, bool> tp {3, 1};

    auto eeprom = tos::open(tos::devs::eeprom<0>);
    int num = 0;
    eeprom->read(0, &num, sizeof num);

    println(comm, "Hello", tos::get<0>(tp), tos::get<1>(tp));
    while (true)
    {
        pinsem.wait();
        ++num;
        eeprom->write(0, &num, sizeof num);
        println(comm, "Pin Change!", num);
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