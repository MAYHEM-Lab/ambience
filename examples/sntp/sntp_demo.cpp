#include <tos/sntp.hpp>
#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <iostream>

void sntp_task() {
    tos::hosted::stdio io;
    tos::hosted::udp_socket sock(get_io());
    sock.send_to(tos::raw_cast(tos::span("hello")), {
        .addr = tos::parse_ipv4_address("127.0.0.1"),
        .port = {9993}
    });
    tos::udp_endpoint_t from;
    uint8_t arr[128];
    auto res = sock.receive_from(arr, from);
    tos::println(io, tos::raw_cast<const char>(force_get(res)));
    auto time = tos::get_sntp_time(sock, {
        .addr = tos::parse_ipv4_address("128.111.1.5"),
        .port = 123
    });
    tos::println(io, time.unix_seconds);
}

void tos_main() {
    tos::launch(tos::alloc_stack, sntp_task);
}