//
// Created by fatih on 1/3/19.
//

#include <tos/ft.hpp>
#include <arch/stm32/drivers.hpp>
#include <tos/print.hpp>
#include <common/dht22.hpp>
#include <common/xbee.hpp>
#include <tos/mem_stream.hpp>
#include "app.hpp"

auto delay = [](std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i)
    {
        __asm__ __volatile__ ("nop");
    }
};

auto setup_usart1 = [](auto& g){
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
};

auto setup_usart2 = [](auto& g){
    using namespace tos::tos_literals;

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_USART3_REMAP_PARTIAL_REMAP;

    auto tx_pin = 42_pin;
    auto rx_pin = 43_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
    gpio_set_mode(GPIO_BANK_USART3_PR_TX, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART3_PR_TX);
};

auto setup_usart0 = [](auto&){
};

void master_task()
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto g = tos::open(tos::devs::gpio);

    setup_usart1(g);
    auto log = tos::open(tos::devs::usart<1>, tos::uart::default_9600);
    setup_usart2(g);
    auto xbee_ser = tos::open(tos::devs::usart<2>, tos::uart::default_9600);
    setup_usart0(g);
    auto slave = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    auto dht = tos::make_dht(g, delay);

    while (true) {
        auto res = dht.read(8_pin);
        if (res != tos::dht_res::ok) {
            tos::println(log, "dht failed");
            goto sleep;
        }

        tos::println(log, "dht", int(dht.temperature), int(dht.humidity));

        namespace xbee = tos::xbee;
        /*g.write(7_pin, tos::digital::high);

        alarm.sleep_for(100ms); // let it wake up
        auto r = xbee::read_modem_status(xbee_ser, alarm);

        if (!r)
        {
            tos::println(log, "xbee non responsive");
            goto sleep;
        }*/

        { // inner block to limit visibility of buff
            auto sample = temp::sample{dht.temperature, dht.humidity, -1};

            static std::array<char, 100> msg_buf;
            tos::omemory_stream buff{msg_buf};

            sample.temp = temp::to_fahrenheits(sample.temp);
            sample.cpu = temp::to_fahrenheits(sample.cpu);

            temp::print(buff, temp::master_id, sample);

            constexpr xbee::addr_16 base_addr{0x0010};
            xbee::tx16_req req{base_addr, tos::raw_cast<const uint8_t>(buff.get()), xbee::frame_id_t{1}};
            xbee::write_to(xbee_ser, req);

            alarm.sleep_for(100ms);
            auto tx_r = xbee::read_tx_status(xbee_ser, alarm);

            if (tx_r) {
                auto &res = tos::force_get(tx_r);
                if (res.status == xbee::tx_status::statuses::success) {
                    tos::println(log, "xbee sent!");
                } else {
                    tos::println(log, "xbee send failed");
                }
            } else {
                tos::println(log, "xbee non responsive");
            }
        } // inner block

        //g.write(7_pin, tos::digital::low);
    sleep:
        alarm.sleep_for(15s);
    }
}

static std::array<char, 2048> stack;
void tos_main()
{
    tos::launch(tos::span<char>(stack), master_task);
}
