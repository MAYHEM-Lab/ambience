//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#include <tvm/exec/executor.hpp>

#include <tos/print.hpp>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>

#include <tos/semaphore.hpp>
#include <drivers/common/eeprom.hpp>

#include "vm_def.hpp"
#include "vm_state.hpp"

#if defined(TOS_ARCH_avr)
#include <arch/avr/usart.hpp>
#include <arch/avr/eeprom.hpp>

#elif defined(TOS_ARCH_lx106)
#include <arch/lx106/usart.hpp>
#endif

#include <arch/lx106/tcp.hpp>
#include <arch/lx106/wifi.hpp>
#include <arch/lx106/tcp.hpp>
#include <common/inet/tcp_stream.hpp>

extern "C"
{
#include <mem.h>
}

struct ptr_fetcher
{
    constexpr uint32_t fetch(uint16_t pc);
    uint8_t* m_base;
};

constexpr uint32_t ptr_fetcher::fetch(uint16_t pc)
{
    auto p = m_base + pc;
    uint32_t res = 0;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++;
    return res;
}

static char tvm_stack[128];

static ptr_fetcher fetch;
static tos::semaphore wait{0};

static svm::vm_state state;
static tos::semaphore s_wait{0};

void tvm_task()
{
    while (true)
    {
        wait.down();

        tvm::vm_executor<ptr_fetcher, svm::vm_state, svm::ISA> exec(fetch);
        exec.m_state.stack_begin = (uint16_t*)tvm_stack;
        exec.m_state.stack_cur = (uint16_t*)tvm_stack;
        exec.m_state.stack_end = (uint16_t*)(tvm_stack + 128);
        exec.exec();

        state = exec.m_state;
        s_wait.up();
    }
}

static uint8_t prog_index = -1;
static uint8_t prog[128];

void main_task()
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(19200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto uart = open(tos::devs::usart<0>, usconf);

    conn:
    tos::esp82::wifi w;
    auto res = w.connect("WIFI", "PASS");

    tos::println(uart, "connected?", res);

    if (!res) goto conn;

    while (!w.wait_for_dhcp());

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(uart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 80 } };
    //auto eeprom = open(tos::devs::eeprom<0>);

    tos::println(*uart, "hello");

    tos::tcp_stream* str = nullptr;
    tos::semaphore conn {0};
    auto acceptor = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint new_ep){
        auto mem = os_malloc(sizeof(tos::tcp_stream));
        str = new (mem) tos::tcp_stream(std::move(new_ep));
        conn.up_isr();
    };
    sock.accept(acceptor);

    while (true)
    {
        conn.down();
        tos::println(*str, "Welcome!");

        char buffer[1];
        auto rd = str->read(buffer);

        if (rd[0] == 'x')
        {
            tos::println(*uart, "execute", rd);
            rd = str->read(buffer);
            tos::println(*uart, rd);

            auto wi = uint8_t(rd[0] - '0');
            tos::println(*uart, "partition:", wi);
            if (prog_index != wi);
            {
                //eeprom->read(wi * 128, prog, 128);
                prog_index = wi;
            }

            fetch = ptr_fetcher{prog};
            wait.up();

            s_wait.down();
            for (auto& reg : state.registers)
            {
                tos::print(*str, reg, " ");
            }
            tos::println(*str, "");
        }
        else if (rd[0] == 'p')
        {
            tos::println(*uart, "program", rd);
            tos::println(*str, "send");
            rd = str->read(buffer);
            tos::println(*uart, rd);

            auto wi = uint8_t(rd[0] - '0');
            rd = str->read(buffer);

            if (uint8_t (rd[0]) > 128)
            {
                tos::println(*str, "too large");
                continue;
            }

            tos::println(*str, "ok");
            str->read({ (char*)prog, rd[0] });
            prog_index = wi;
            //eeprom->write(wi * 128, prog, 128);
            tos::println(*str, "okay");
        }

        tos::std::destroy_at(str);
        os_free(str);
    }
}

void tos_main() {
    tos::launch(tvm_task);
    tos::launch(main_task);
}
