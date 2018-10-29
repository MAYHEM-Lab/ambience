//
// Created by Mehmet Fatih BAKIR on 11/10/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <drivers/common/xbee.hpp>
#include <drivers/common/alarm.hpp>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

void tx_task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto gpio = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<1>);
    auto alarm = tos::open(tos::devs::alarm, *tmr);

    while (true)
    {
        using namespace std::chrono_literals;

        namespace xbee = tos::xbee;

        gpio.write(7_pin, tos::digital::high);
        alarm.sleep_for(50ms);

        tos::xbee_s1<tos::avr::usart0> x {usart};
        xbee::frame_id_t fid{1};

        constexpr xbee::addr_16 base_addr { 0xABCD };
        std::array<uint8_t, 2> buf = { 'h', 'i' };
        xbee::tx16_req r { base_addr, buf, fid };

        x.transmit(r);
        fid.id++;

        alarm.sleep_for(20ms);
        gpio.write(7_pin, tos::digital::low);

        alarm.sleep_for(1s);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}