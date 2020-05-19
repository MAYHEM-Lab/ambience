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

namespace tos::lwip {
class basic_interface {
public:
    basic_interface(const ipv4_addr_t& addr,
                    const ipv4_addr_t& mask,
                    const ipv4_addr_t& gw)
        : m_tap(std::move(force_get(hosted::make_tap_device()))) {
        auto lwip_addr = convert_address(addr);
        auto lwip_mask = convert_address(mask);
        auto lwip_gw = convert_address(gw);
        netif_add(&m_if,
                  &lwip_addr,
                  &lwip_mask,
                  &lwip_gw,
                  this,
                  &basic_interface::init,
                  netif_input);
    }

    void up() {
        netif_set_link_up(&m_if);
        netif_set_up(&m_if);
    }

    void down() {
        netif_set_down(&m_if);
        netif_set_link_down(&m_if);
    }

    ~basic_interface() {
        down();
        netif_remove(&m_if);
    }

private:
    netif m_if;
    hosted::tap_device m_tap;

    err_t init() {
        m_if.hostname = "tos";
        m_if.flags |= NETIF_FLAG_ETHARP | NETIF_FLAG_BROADCAST;
        m_if.linkoutput = &basic_interface::link_output;
        m_if.output = etharp_output;
        m_if.name[1] = m_if.name[0] = 'm';
        m_if.num = 0;
        m_if.mtu = 1500;
        m_if.hwaddr_len = 6;
        m_if.link_callback = &basic_interface::link_callback;
        m_if.status_callback = &basic_interface::status_callback;
        std::iota(std::begin(m_if.hwaddr), std::end(m_if.hwaddr), 1);
        launch(alloc_stack, [this] { read_thread(); });
        return ERR_OK;
    }

    void read_thread() {
        while (true) {
            std::vector<uint8_t> buf(1500);
            auto recvd = m_tap.read(buf);
            LOG_TRACE("Received", recvd.size(), "bytes");
            auto p =
                pbuf_alloc(pbuf_layer::PBUF_LINK, recvd.size(), pbuf_type::PBUF_POOL);
            std::copy(recvd.begin(), recvd.end(), static_cast<uint8_t*>(p->payload));
            m_if.input(p, &m_if);
        }
    }

    err_t link_output(pbuf* p) {
        LOG_TRACE("link_output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        auto written = m_tap.write({static_cast<const uint8_t*>(p->payload), p->len});
        LOG_TRACE("Written", written, "bytes");
        return ERR_OK;
    }

    err_t output(pbuf* p, const ip4_addr_t* ipaddr) {
        LOG_TRACE("output", p->len, "bytes");
        // pbuf_ref(p);
        // return m_if.input(p, &m_if);
        return ERR_OK;
    }

    void link_callback() {
    }

    void status_callback() {
    }

private:
    static err_t init(struct netif* netif) {
        return static_cast<basic_interface*>(netif->state)->init();
    }

    static err_t link_output(struct netif* netif, struct pbuf* p) {
        return static_cast<basic_interface*>(netif->state)->link_output(p);
    }

    static err_t output(struct netif* netif, struct pbuf* p, const ip4_addr_t* ipaddr) {
        return static_cast<basic_interface*>(netif->state)->output(p, ipaddr);
    }

    static void link_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->link_callback();
    }

    static void status_callback(struct netif* netif) {
        static_cast<basic_interface*>(netif->state)->status_callback();
    }

    friend void set_default(basic_interface& interface) {
        netif_set_default(&interface.m_if);
    }
};
} // namespace tos::lwip

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

    LOG("Initialize lwip");
    lwip_init();
    tos::lwip::basic_interface interface(tos::parse_ipv4_address("192.168.0.10"),
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