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

auto setup_usart0 = [](auto& g){
    using namespace tos::tos_literals;

    auto tx_pin = 9_pin;
    auto rx_pin = 10_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);
};

auto xbee_task = [](auto& g, auto& log)
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;
    namespace xbee = tos::xbee;

    auto tmr = tos::open(tos::devs::timer<2>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    setup_usart0(g);
    auto xbee_ser = tos::open(tos::devs::usart<0>, tos::uart::default_9600);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

    auto dht_pin = 1_pin;
    g.set_pin_mode(dht_pin, tos::pin_mode::in_pullup);

    auto d = tos::make_dht(g, delay);

    g.set_pin_mode(11_pin, tos::pin_mode::out);
    g.write(11_pin, tos::digital::low);
    alarm.sleep_for(100ms); // let it die

    while (true)
    {
        int tries = 1;
        auto res = d.read(dht_pin);

        while (res != tos::dht_res::ok && tries < 5) {
            tos::println(log, "dht non-responsive");
            alarm.sleep_for(2s);
            res = d.read(dht_pin);
            ++tries;
        }

        if (tries == 5) {
            d.temperature = -1;
            d.humidity = -1;
        }

        std::array<temp::sample, 2> samples = {
                temp::sample{d.temperature, d.humidity, 0},
                {}
        };

        char buf[50];
        tos::omemory_stream str{buf};

        temp::print(str, 102, samples[0]);

        g.write(11_pin, tos::digital::high);

        alarm.sleep_for(100ms); // let it wake up
        auto r = xbee::read_modem_status(xbee_ser, alarm);

        if (!r)
        {
            tos::println(log, "xbee non responsive");
        }

        constexpr xbee::addr_16 base_addr{0x0010};

        xbee::tx16_req req{base_addr, tos::raw_cast<const uint8_t>(str.get()), xbee::frame_id_t{0xAB}};
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

        g.write(11_pin, tos::digital::low);
        alarm.sleep_for(5s);
    }

    tos::this_thread::block_forever();
};

tos::kern::tcb* tcb;
static std::array<char, 1024> sstack alignas(std::max_align_t);
void master_task()
{
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    namespace xbee = tos::xbee;

    auto g = tos::open(tos::devs::gpio);

    //setup_usart1(g);
    //auto log = tos::open(tos::devs::usart<1>, tos::uart::default_9600);
    //setup_usart2(g);
    //auto xbee_ser = tos::open(tos::devs::usart<2>, tos::uart::default_9600);
    //setup_usart0(g);
    //auto slave = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    struct x : tos::self_pointing<x> {
        int write(tos::span<const char> x) { return x.size(); }
    } l;

    //xbee_task(g, log);
    //return;
    tos::semaphore s{0};
    auto& t = tos::launch(tos::span<char>(sstack), [&]{
        xbee_task(g, l);
       s.up();
    });
    tcb = &t;
    s.down();
}

static std::array<char, 1024> stack;
void tos_main()
{
    tos::launch(tos::span<char>(stack), master_task);
}
