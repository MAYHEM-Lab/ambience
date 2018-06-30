//
// Created by fatih on 4/26/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/utility.hpp>
#include <tos/memory.hpp>

#include <drivers/arch/lx106/timer.hpp>
#include <drivers/arch/lx106/usart.hpp>
#include <drivers/arch/lx106/wifi.hpp>
#include <drivers/arch/lx106/tcp.hpp>

extern "C"
{
#include <mem.h>
}

namespace tos
{
    class tcp_stream
    {
    public:

        explicit tcp_stream(tos::esp82::tcp_endpoint&& ep);

        void write(span<const char>);

        span<char> read(span<char>);

        void operator()(esp82::events::sent_t, esp82::tcp_endpoint&);
        void operator()(esp82::events::recv_t, esp82::tcp_endpoint&, span<const char>);

    private:

        void attach();

        tos::fixed_fifo<char, 512> m_fifo;

        tos::esp82::tcp_endpoint m_ep;
        tos::mutex m_busy;
        tos::semaphore m_write_sync{0};
        tos::event m_read_sync;

        tos::span<char>::iterator m_it{};
        tos::span<char>::iterator m_end{};
    };

    tcp_stream::tcp_stream(tos::esp82::tcp_endpoint &&ep)
            : m_ep(std::move(ep)) {
        attach();
    }

    void tcp_stream::attach() {
        m_ep.attach(esp82::events::recv, *this);
        m_ep.attach(esp82::events::sent, *this);
    }

    void tcp_stream::operator()(tos::esp82::events::sent_t, tos::esp82::tcp_endpoint &) {
        m_write_sync.up();
    }

    void tcp_stream::write(tos::span<const char> buf) {
        tos::lock_guard<tos::mutex> lk{ m_busy };
        m_ep.send(buf);
        m_write_sync.down();
    }

    void tcp_stream::operator()(esp82::events::recv_t, esp82::tcp_endpoint &, span<const char> buf) {
        auto it = buf.begin();
        auto end = buf.end();

        while (it != end && m_it != m_end)
        {
            *m_it++ = *it++;
        }

        while (it != end && m_fifo.push_isr(*it++));

        m_read_sync.fire_isr();
    }

    span<char> tcp_stream::read(tos::span<char> to) {
        tos::lock_guard<tos::mutex> lk{ m_busy };

        {
            tos::int_guard ig;
            auto it = to.begin();
            auto end = to.end();

            while (m_fifo.size() > 0 && it != end)
            {
                *it++ = m_fifo.pop();
            }

            m_it = it;
            m_end = end;
        }

        if (m_it != m_end)
        {
            m_read_sync.wait();
        }

        return to.slice(0, m_it - to.begin());
    }
}

tos::tcp_stream* socket;
char buf[512];
void socket_task()
{
    auto req = socket->read(buf);
    tos::println(*socket, "HTTP/1.0 200 Content-type: text/html");
    tos::println(*socket);
    tos::print(*socket, "<body><b>Hello from Tos!</b><br/><code>");
    tos::print(*socket, req);
    tos::println(*socket, "</code></body>");
    tos::println(*socket);

    tos::std::destroy_at(socket);
    os_free(socket);
}

void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    tos::print(*usart, "\n\n\n\n\n\n");

    tos::esp82::wifi w;
    auto res = w.connect("WIFI", "PASS");

    tos::println(*usart, "connected?", res);

    while (!w.wait_for_dhcp());

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(*usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 80 } };
    tos::fixed_fifo<int, 24> buf;

    auto handler = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint new_ep){
        auto mem = os_malloc(sizeof(tos::tcp_stream));
        socket = new (mem) tos::tcp_stream(std::move(new_ep));
        tos::launch(socket_task);
    };

    sock.accept(handler);

    buf.push(100);
    tos::println(*usart, tos::platform::board_name);

    while (true)
    {
        auto c = buf.pop();
        tos::println(*usart, c);
    }
}

void tos_main()
{
    tos::launch(task);
}