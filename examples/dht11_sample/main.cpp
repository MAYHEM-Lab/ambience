#include <arch/drivers.hpp>
#include <common/dht22.hpp>
#include <common/inet/tcp_stream.hpp>
#include <cwpack.hpp>
#include <lwip/init.h>
#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>
#include <tos/utility.hpp>

auto gpio = open(tos::devs::gpio);
auto dht =
    tos::make_dht(gpio, [](std::chrono::microseconds us) { os_delay_us(us.count()); });
void send(tos::esp82::wifi_connection& c) {
    auto e_res = tos::esp82::connect(c, {178, 62, 54, 64}, {4242});
    if (!e_res) {
        return;
    }

    auto res = dht.read(tos::esp82::pin_t{4});
    // tos::println(usart, int(res), int(dht.temperature), int(dht.humidity));
    static uint8_t buf[128];
    tos::msgpack::packer p{buf};
    auto arr = p.insert_arr(2);
    arr.insert(dht.temperature);
    arr.insert(dht.humidity);

    auto& conn = force_get(e_res);
    tos::tcp_stream<tos::esp82::tcp_endpoint> stream(std::move(conn));

    stream.write(p.get());
}

char buf[512];
void task() {
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
                                .add(115200_baud_rate)
                                .add(tos::usart_parity::disabled)
                                .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);

    tos::esp82::wifi w;

conn_:
    auto res = w.connect("cs190b", "cs190bcs190b");

    tos::println(usart, "connected?", bool(res));
    if (!res)
        goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    auto addr = force_get(wconn.get_addr());

    tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);

    lwip_init();

    auto tmr = open(tos::devs::timer<0>);
    tos::alarm alarm(&tmr);

    while (true) {
        using namespace std::chrono_literals;
        send(wconn);
        system_deep_sleep_set_option(0);
        system_deep_sleep_instant(30'000'000);
        // tos::this_thread::sleep_for(alarm, 30s);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
