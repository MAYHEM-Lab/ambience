//
// Created by fatih on 11/8/18.
//

#include <drivers/common/ina219.hpp>
#include <drivers/arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/print.hpp>

void main_task(void*)
{
    using namespace tos;
    using namespace tos_literals;

    avr::twim t{17_pin, 18_pin};
    ina219<avr::twim> ina{ {0x40}, t };

    ina.setCalibration_32V_2A();

    auto tmr = open(devs::timer<1>);
    auto alarm = open(devs::alarm, tmr);

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(500ms);
        int curr = ina.getCurrent_mA();
        int v = ina.getBusVoltage_raw();

        tos::println(usart, "V:", v);
        tos::println(usart, "I:", curr, "mA");
    }
}

void tos_main()
{
    tos::launch(main_task);
}