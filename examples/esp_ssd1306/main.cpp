//
// Created by fatih on 12/23/18.
//

#include <arch/drivers.hpp>
#include <common/adxl345.hpp>
#include <common/gpio.hpp>
#include <common/inet/tcp_stream.hpp>
#include <common/ssd1306.hpp>
#include <tos/ft.hpp>
#include <tos/gfx/canvas.hpp>
#include <tos/gfx/text.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

constexpr auto font = tos::gfx::basic_font().rotate_90_cw().rotate_90_cw().rotate_90_cw();

auto wifi_connect() {
    tos::esp82::wifi w;

conn_:
    auto res = w.connect("SSID", "password");
    if (!res)
        goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    return std::make_pair(w, std::move(wconn));
}

auto task = [] {
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto gpio = tos::open(tos::devs::gpio);

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::this_thread::sleep_for(alarm, 100ms);

    auto i2c = tos::open(
        tos::devs::i2c<0>, tos::i2c_type::master, gpio, gpio.port.pin4, gpio.port.pin5);

    tos::ssd1306 oled(&i2c, {0x3C}, 128, 64);
    oled.dim(false);

    draw_text(oled,
              font,
              "Hello from tos",
              tos::gfx::point{0, 0},
              tos::gfx::text_direction::vertical);

    oled.display();

    auto wifi = wifi_connect();

    draw_text(oled,
              font,
              "Wifi Connected",
              tos::gfx::point{12, 0},
              tos::gfx::text_direction::vertical);

    oled.display();

    std::array<char, 20> buf;
    tos::omemory_stream str(buf);

    tos::ipv4_addr_t ip = force_get(wifi.second.get_addr());
    tos::print(str,
               int(ip.addr[0]),
               int(ip.addr[1]),
               int(ip.addr[2]),
               int(ip.addr[3]),
               tos::separator('.'));

    draw_text(oled,
              font,
              str.get(),
              tos::gfx::point{24, 0},
              tos::gfx::text_direction::vertical);

    oled.display();

    auto last = system_get_time();
    while (true)
    {
        tos::omemory_stream str(buf);
        auto now = system_get_time();
        tos::print(str, int(now - last));

        last = system_get_time();
        draw_text(oled,
                  font,
                  str.get(),
                  tos::gfx::point{36, 0},
                  tos::gfx::text_direction::vertical);

        oled.display();
    }

    tos::this_thread::block_forever();
};

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
