//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <arch/drivers.hpp>
#include <common/alarm.hpp>
#include <common/epd/waveshare/bw29.hpp>

#include <nrf_delay.h>

#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/fixed_fifo.hpp>

#include "text.hpp"
#include "canvas.hpp"
#include "bakir_ble.hpp"

void gap_params_init();
void gatt_init();

bakir::canvas<128, 296> framebuf;
constexpr auto font =
    bakir::basic_font()
        .inverted()             // black on white
        .mirror_horizontal()    // left to right
        .rotate_90_cw();        // screen is rotated

tos::fixed_fifo<char, 2> cmds;

auto epd_task = []{
    using namespace tos;
    using namespace tos_literals;
    auto g = open(tos::devs::gpio);

    auto mosi = 33_pin;
    auto clk = 34_pin;
    auto cs = 35_pin;
    auto dc = 36_pin;
    auto reset = 37_pin;
    auto busy = 38_pin;

    g->set_pin_mode(cs, tos::pin_mode::out);
    g->write(cs, tos::digital::high);

    {
        framebuf.fill(true);
        auto print_str = [&](auto& str, size_t x, size_t y) {
            for (char c : str) {
                if (c == 0) return;
                auto ch = font.get(c);
                if (!ch)
                {
                    ch = font.get('?');
                }
                framebuf.copy(*ch, x, y);
                y += ch->height();
            }
        };
        print_str("Hello world", 96, 24);

        tos::nrf52::spi s(clk, std::nullopt, mosi);
        epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
        epd.initialize([](std::chrono::milliseconds ms) {
            nrf_delay_ms(ms.count());
        });
        epd.SetFrameMemory(framebuf.data(), 0, 0, epd.width, epd.height);
        epd.DisplayFrame();
    }

    while (true)
    {
        auto c = cmds.pop();

        if (c == 'a')
        {
            tos::nrf52::spi s(clk, std::nullopt, mosi);

            epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
            epd.initialize([](std::chrono::milliseconds ms) {
                nrf_delay_ms(ms.count());
            });

            epd.SetFrameMemory(framebuf.data(), 0, 0, epd.width, epd.height);
            epd.DisplayFrame();
        }
        else if (c == 'c')
        {
            tos::nrf52::spi s(clk, std::nullopt, mosi);

            epd<decltype(&s)> epd(&s, cs, dc, reset, busy);
            epd.initialize([](std::chrono::milliseconds ms) {
                nrf_delay_ms(ms.count());
            });

            epd.ClearFrameMemory(0xFF);
            epd.DisplayFrame();
        }
    }
};

auto ble_task = [](bool have_epd)
{
    using namespace tos;
    using namespace tos_literals;
    constexpr auto usconf = tos::usart_config()
        .add(115200_baud_rate)
        .add(usart_parity::disabled)
        .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "hello");

    tos::nrf52::softdev sd;
    auto setname_res = sd.set_device_name("Tos BLE");
    auto tx_pow_res = sd.set_tx_power();
    tos::println(usart, "sd initd", bool(setname_res));
    gap_params_init();
    tos::println(usart, "gap initd");
    gatt_init();
    tos::println(usart, "gatt initd");
    auto serv = bakir::make_ble_service(sd, usart);
    tos::println(usart, "services initd");
    tos::nrf52::advertising adv;
    tos::println(usart, "adv initd");

    auto started = adv.start();
    if (started)
    {
        tos::println(usart, "began adv");
    }
    else
    {
        tos::println(usart, "adv failed");
    }

    if (have_epd)
    {
        tos::println(usart, "Have epaper, initializing display task");
        tos::launch(alloc_stack, epd_task);
    }

    using namespace std::chrono_literals;
    while (true)
    {
        std::array<char, 1> c;
        auto r = serv->read(c);

        if (have_epd)
        {
            cmds.push(r[0]);
        }

        usart->write(r);
    }

    tos::this_thread::block_forever();
};

void TOS_EXPORT tos_main()
{
    tos::launch(tos::alloc_stack, ble_task, true);
}
