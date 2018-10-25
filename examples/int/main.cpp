//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <tos/devices.hpp>
#include <tos/interrupt.hpp>
#include <tos/semaphore.hpp>
#include <tos/event.hpp>

#include <tuple>

void tick_task(void*)
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event pinsem;
    auto handler = [&]{
        pinsem.fire();
    };

    g->attach_interrupt(2_pin, tos::pin_change::falling, handler);

    auto eeprom = tos::open(tos::devs::eeprom<0>);
    int num = 0;
    eeprom->read(0, &num, sizeof num);

    while (true)
    {
        pinsem.wait();
        ++num;
        eeprom->write(0, &num, sizeof num);
        tos::println(*usart, "Pin Change!", num);
    }
}

void tos_main()
{
    tos::launch(tick_task);
}