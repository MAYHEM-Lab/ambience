//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <tos/devices.hpp>
#include <tos/interrupt.hpp>
#include <tos/semaphore.hpp>
#include <tos/event.hpp>

#include <tuple>

void tick_task()
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
    char numb[sizeof(int)];
    eeprom->read(0, numb);

    while (true)
    {
        pinsem.wait();
        int num;
        std::memcpy(&num, numb, sizeof num);
        ++num;
        std::memcpy(numb, &num, sizeof num);
        eeprom->write(0, numb);
        tos::println(*usart, "Pin Change!", num);
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, tick_task);
}