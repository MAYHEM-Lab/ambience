//
// Created by fatih on 10/31/19.
//

#include "tos/stack_storage.hpp"

#include <arch/drivers.hpp>
#include <arch/tcp.hpp>
#include <arch/udp.hpp>
#include <arch/wlan.hpp>
#include <common/inet/tcp_ip.hpp>
#include <common/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/streams.hpp>

static tos::any_usart* uart;

void udp_socket() {
    tos::cc32xx::udp_socket sock;
    sock.bind({5001});

    sock.send_to(tos::raw_cast<const uint8_t>(tos::span<const char>("hello")),
                 {.addr = {{192, 168, 43, 120}}, .port = 5001});

    while (true) {
        std::array<uint8_t, 32> buffer;
        tos::udp_endpoint_t from;
        auto res = sock.receive_from(buffer, from);
        tos::println(uart, "Received:", tos::raw_cast<const char>(force_get(res)));
    }
}

tos::intrusive_ptr<tos::cc32xx::tcp_socket> get_connection() {
    auto listener = tos::make_intrusive<tos::cc32xx::tcp_listener>(tos::port_num_t{8080});
    listener->listen();
    auto sock = listener->accept();
    tos::println(uart, "accept returned", bool(sock));
    if (!sock) {
        tos::println(uart, "accept error!");
        return nullptr;
    }
    return std::move(force_get(sock));
}

void tcp_socket() {
    auto socket_ptr = get_connection();
    tos::println(uart, "socket:", socket_ptr->native_handle());
    std::array<char, 32> buffer;
    auto line = tos::read_until<char>(socket_ptr, "\n", buffer);
    tos::println(uart, "Socket received:", line);
    socket_ptr->write(tos::raw_cast<const uint8_t>(line));
}

tos::any_usart* log;

class ev_handler : public tos::cc32xx::iwifi_event_handler {
public:
    void handle(const tos::cc32xx::ip_acquired& acquired) override {
        tos::println(uart, "IP Acquired!");
        tos::launch(tos::alloc_stack, ::tcp_socket);
    }
    void handle(const tos::cc32xx::wifi_connected& connected) override {
    }
    void handle(const tos::cc32xx::wifi_disconnected& disconnected) override {
    }
};

void task() {
    using namespace tos::tos_literals;
    auto pin = 4_pin;
    tos::cc32xx::gpio g;

    g.set_pin_mode(5_pin, tos::pin_mode::out);
    g.set_pin_mode(6_pin, tos::pin_mode::out);
    g.set_pin_mode(pin, tos::pin_mode::out);
    g.write(pin, tos::digital::high);

    tos::cc32xx::uart uart(0);
    auto erased = tos::erase_usart(&uart);
    ::uart = &erased;
    ::log = &erased;

    tos::println(uart);

    tos::cc32xx::simplelink_wifi wifi;
    wifi.set_event_handler(*new ev_handler);
    wifi.connect("SSID", "PASS");

    tos::cc32xx::timer tim(0);
    tos::alarm alarm(&tim);

    while (true) {
        using namespace std::chrono_literals;
        g.write(pin, tos::digital::high);
        // tos::println(uart, "high");
        tos::this_thread::sleep_for(alarm, 1000ms);
        g.write(pin, tos::digital::low);
        // tos::println(uart, "low");
        tos::this_thread::sleep_for(alarm, 1000ms);
    }
}

void flash_task() {
    tos::cc32xx::uart uart(0);
    tos::println(uart, "hello");
    tos::cc32xx::flash flash;
    auto res = flash.erase(255);
    tos::println(uart, "done", bool(res));
    uintptr_t addr = 0x01000000 + 255 * flash.sector_size_bytes();
    auto x = reinterpret_cast<uint32_t*>(addr);
    tos::println(uart, *x);
    uint32_t data = 42;
    res = flash.write(255, tos::raw_cast(tos::monospan(data)), 0);
    tos::println(uart, "done", bool(res));
    tos::println(uart, *x);

    while (true) {
        asm volatile("nop");
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, flash_task);
    // tos::launch(tos::alloc_stack, task);
}
