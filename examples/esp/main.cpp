//
// Created by fatih on 4/26/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>
#include <tos/memory.hpp>
#include <common/inet/tcp_stream.hpp>

#include <arch/lx106/timer.hpp>
#include <arch/lx106/usart.hpp>
#include <arch/lx106/wifi.hpp>
#include <arch/lx106/tcp.hpp>
#include <tos/version.hpp>

#include <lwip/tcp.h>

extern "C"
{
#include <mem.h>
}

volatile bool running = false;
alignas(alignof(tos::esp82::tcp_endpoint)) char mem[sizeof(tos::esp82::tcp_endpoint)];

tos::esp82::tcp_endpoint* socket;
char sock_stack[8192];
tos::esp82::uart0* u;

tos::function_ref<void(const char*)> print_debug{ [](const char*, void*){} };
volatile int x = 0;
volatile int cnt = 0;

struct stack_check
{
public:
    stack_check(void* begin, size_t sz)
        : m_begin(reinterpret_cast<uint32_t*>(begin)),
          m_cnt{sz / sizeof(m_marker)},
          m_marker{0xDEADBEEF}
    {
        for (int i = 0; i < m_cnt; ++i)
        {
            m_begin[i] = m_marker;
        }
    }

    size_t get_usage() const
    {
        // stack grows downwards, so count from the 0th index, but subtract from the top!
        for (int i = 0; i < m_cnt; ++i)
        {
            if (m_begin[i] != m_marker)
            {
                return (m_cnt - i) * sizeof m_marker;
            }
        }
        return 0;
    }

private:
    uint32_t * m_begin;
    size_t m_cnt;
    uint32_t m_marker;
};

stack_check* sc;

void socket_task()
{
    //tos::esp82::prnt_debug(*u);
    //auto req = socket->read(buf);
    tos::semaphore s{0};
    cnt = 0;

    char buf[512];
    auto recv_handler = [&](auto, tos::esp82::tcp_endpoint&, tos::span<const char> data){
        if (data.data())
        {
            std::copy(data.begin(), data.end(), buf);
            buf[data.size()] = 0;
        }
        s.up_isr();
        x++;
        cnt++;
    };

    auto sent_handler = [&](auto, tos::esp82::tcp_endpoint&){
        s.up_isr();
    };

    socket->attach(tos::esp82::events::recv, recv_handler);
    socket->attach(tos::esp82::events::sent, sent_handler);

    s.down();

    socket->send("hello\r\n\r\n");
    s.down();

    socket->send(buf);
    s.down();

    tos::std::destroy_at(socket);
    running = false;
    tos::println(*u, "done", x, cnt, int(sc->get_usage()));
    tos::std::destroy_at(sc);
    os_free(sc);
}

void task()
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);
    usart.enable();
    
    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("Nakedsense.2", "serdar1988");

    tos::println(usart, "connected?", res);
    if (!res) goto conn;

    while (!w.wait_for_dhcp());

    tos::esp82::wifi_connection conn;
    auto addr = conn.get_addr();
    tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);

    tos::esp82::tcp_socket sock{ w, { 80 } };
    tos::fixed_fifo<char, 100> cbuf;

    auto printer = [&](const char* what)
    {
        while (*what)
        {
            cbuf.push_isr(*what++);
        }
    };

    u = &usart;
    print_debug = printer;

    auto handler = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint&& new_ep){
        //printer("got conn");

        if (running){
            printer("running");
            return;
        }
        running = true;

        socket = new (mem) tos::esp82::tcp_endpoint(std::move(new_ep));

        auto m = os_malloc(sizeof(stack_check));
        sc = new (m) stack_check(sock_stack, 8192);

        constexpr auto params = tos::thread_params()
                .add<tos::tags::entry_pt_t>(&socket_task)
                .add<tos::tags::stack_ptr_t>((void*)sock_stack)
                .add<tos::tags::stack_sz_t>(8192U);

        tos::launch(params);
    };

    sock.accept(handler);
    print_debug("hi");

    while (true)
    {
        auto x = cbuf.pop();
        tos::print(usart, x);
    }

    tos::semaphore s{0};
    s.down(); // block forever
}

void tos_main()
{
    tos::launch(task);
}