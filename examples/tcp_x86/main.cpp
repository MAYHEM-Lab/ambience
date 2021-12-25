#include <arch/drivers.hpp>
#include <tos/ft.hpp>

void tcp_task() {
    auto uart = tos::open(tos::devs::usart<0>, tos::uart::default_115200);

    auto conn = tos::hosted::connect(tos::parse_ipv4_address("93.184.216.34"), {80});
    if (!conn) {
        return;
    }
    auto& sock = force_get(conn);
    tos::println(sock, "GET / HTTP/1.1");
    tos::println(sock, "Host: example.com");
    tos::println(sock, "Connection: close");
    tos::println(sock);

    while (true) {
        std::array<uint8_t, 64> buf;
        auto res = sock->read(buf);
        if (!res) {
            return;
        }
        tos::print(uart, tos::raw_cast<const char>(force_get(res)));
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, tcp_task);
}