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
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);
    usart.enable();

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    namespace xbee = tos::xbee;

    tos::xbee_s1<tos::avr::usart0> x {tos::std::move(usart)};

    auto tmr = tos::open(tos::devs::timer<1>);
    auto alarm = tos::open(tos::devs::alarm, *tmr);

    while (true)
    {
        using namespace tos::chrono_literals;

        xbee::addr_16 base_addr { 0x1234 } ;
        uint8_t buf[] = { 'h', 'i' };
        xbee::tx16_req r { base_addr, buf };

        x.transmit(r);

        alarm.sleep_for(1000_ms);
    }
}

void tos_main()
{
    tos::launch(tx_task);
}