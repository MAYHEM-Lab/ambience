//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <arch/drivers.hpp>
#include <tos/arch.hpp>
#include <tos/devices.hpp>
#include <tos/event.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>

void tick_task() {
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    auto g = tos::open(tos::devs::gpio);
    g->set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event pinsem;
    auto handler = [&] { pinsem.fire(); };

    tos::avr::exti external_interrupts;
    external_interrupts->attach(
        2_pin, tos::pin_change::falling, tos::function_ref<void()>(handler));

    auto eeprom = tos::open(tos::devs::eeprom<0>);
    char numb[sizeof(int)];
    eeprom->read(0, numb);

    while (true) {
        pinsem.wait();
        int num;
        std::memcpy(&num, numb, sizeof num);
        ++num;
        std::memcpy(numb, &num, sizeof num);
        eeprom->write(0, numb);
        tos::println(*usart, "Pin Change!", num);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, tick_task);
}