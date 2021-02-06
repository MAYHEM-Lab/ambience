#include <arch/drivers.hpp>
#include <arch/tap.hpp>
#include <common/inet/tcp_ip.hpp>
#include <common/inet/tcp_stream.hpp>
#include <lwip/etharp.h>
#include <lwip/init.h>
#include <lwip/sys.h>
#include <lwip/timeouts.h>
#include <numeric>
#include <tos/debug/assert.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/clock_adapter.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/ft.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/tcp.hpp>
#include <tos/lwip/udp.hpp>
#include <tos/lwip/utility.hpp>

void tcp_task() {
    tos::lwip::tcp_socket sock({80});

    tos::semaphore s{0};
    tos::tcp_stream<tos::lwip::tcp_endpoint>* ep;

    auto acceptor = [&](auto&, tos::lwip::tcp_endpoint&& newep) {
        if (ep) {
            return false;
        }
        ep = new tos::tcp_stream<tos::lwip::tcp_endpoint>(std::move(newep));
        s.up();
        return true;
    };

    sock.async_accept(acceptor);

    int cnt = 0;
    while (true) {
        ep = nullptr;

        s.down();

        LOG("Got connection");

        std::array<uint8_t, 512> buf;
        auto req = force_get(ep->read(buf));
        ++cnt;

        tos::println(ep, "HTTP/1.0 200 Content-type: text/html");
        tos::println(ep);
        tos::print(ep, "<body><b>Hello from Tos!</b><br/><code>");
        ep->write(req);
        tos::println(ep, "</code><br/>");
        tos::println(ep, "<ul>");
        tos::println(ep, "<li>", int(sys_now()), "</li>");
        tos::println(ep, "<li>", cnt, "</li>");
        tos::println(ep, "</ul></body>");
        tos::println(ep);

        LOG("Served request");

        delete ep;
        ep = nullptr;
    }
}

tos::stack_storage tcp_stack;
void lwip_task() {
    auto clk = tos::erase_clock(tos::hosted::clock<std::chrono::system_clock>());
    tos::lwip::global::system_clock = &clk;

    hosted::tap_device tap_dev(std::move(force_get(hosted::make_tap_device())));

    LOG("Initialize lwip");
    lwip_init();
    tos::lwip::basic_interface interface(std::move(tap_dev),
                                         tos::parse_ipv4_address("192.168.0.10"),
                                         tos::parse_ipv4_address("255.255.255.0"),
                                         tos::parse_ipv4_address("192.168.0.1"));
    set_default(interface);
    interface.up();

    tos::lwip::async_udp_socket sock;

    auto handler = [](auto, auto, auto, tos::lwip::buffer buf) {
        LOG("Received", buf.size(), "bytes!");
        std::vector<uint8_t> data(buf.size());
        buf.read(data);
        LOG(data);
    };

    sock.attach(handler);

    auto res = sock.bind({9090}, tos::parse_ipv4_address("192.168.0.10"));
    Assert(res);

    tos::launch(tcp_stack, tcp_task);

    tos::hosted::timer timer{};
    tos::alarm alarm(&timer);
    while (true) {
        sys_check_timeouts();
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 100ms);
    }
    tos::this_thread::block_forever();
}

tos::stack_storage stack;
void tos_main() {
    tos::debug::set_default_log(
        new tos::debug::detail::any_logger(new tos::debug::clock_sink_adapter{
            tos::debug::serial_sink(tos::hosted::stderr_adapter{}),
            tos::hosted::clock<std::chrono::system_clock>{}}));

    tos::launch(stack, lwip_task);
}