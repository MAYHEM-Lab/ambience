//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#include "vm_def.hpp"
#include "vm_state.hpp"

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>
#include <tvm/exec/executor.hpp>

struct ptr_fetcher {
    constexpr uint32_t fetch(uint16_t pc);
    uint8_t* m_base;
};

constexpr uint32_t ptr_fetcher::fetch(uint16_t pc) {
    auto p = m_base + pc;
    uint32_t res = 0;
    res |= *p++;
    res <<= 8;
    res |= *p++;
    res <<= 8;
    res |= *p++;
    res <<= 8;
    res |= *p++;
    return res;
}

static char tvm_stack[128];

static ptr_fetcher fetch;
static tos::semaphore wait{0};

static svm::vm_state state;
static tos::semaphore s_wait{0};

void tvm_task() {
    while (true) {
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
static uint8_t prog[128] = {0x04, 0x00, 0x01, 0x40, 0x04, 0x20, 0x00, 0x00, 0x04, 0x40,
                            0x00, 0x00, 0x08, 0x04, 0x00, 0x3e, 0x02, 0x20, 0x04, 0x5f,
                            0xff, 0xe0, 0x02, 0x04, 0x04, 0x40, 0x00, 0x00, 0x06, 0x00,
                            0x18, 0x0c, 0x02, 0x0a, 0x00, 0x00, 0x00, 0x00};

void main_task() {
    using namespace tos::tos_literals;

    auto uart = open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::println(uart, "hello");
    fetch = ptr_fetcher{prog};
    wait.up();

    s_wait.down();
    for (auto& reg : state.registers) {
        tos::print(uart, reg, " ");
    }
    tos::println(uart, "");

    //    tos::esp82::wifi w;
    //    conn_:
    //    auto res = w.connect("cs190b", "cs190bcs190b");
    //
    //    if (!res) goto conn_;
    //
    //    auto& wconn = force_get(res);
    //
    //    wconn.wait_for_dhcp();
    //
    //    tos::esp82::tcp_socket sock{ wconn, tos::port_num_t{ 80 } };

    //    using sock_ptr = std::unique_ptr<tos::tcp_stream<tos::esp82::tcp_endpoint>>;
    //    sock_ptr str;
    //    tos::semaphore conn {0};
    //    auto acceptor = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint&& new_ep){
    //        str =
    //        std::make_unique<tos::tcp_stream<tos::esp82::tcp_endpoint>>(std::move(new_ep));
    //        conn.up_isr();
    //    };
    //    sock.accept(acceptor);
    //
    //    while (true)
    //    {
    //        conn.down();
    //        tos::println(*str, "Welcome!");
    //
    //        char buffer[1];
    //        auto rdx = str->read(buffer);
    //
    //        if (!rdx) continue;
    //        auto rd = force_get(rdx);
    //
    //        if (rd[0] == 'x')
    //        {
    //            tos::println(*uart, "execute", rd);
    //            rdx = str->read(buffer);
    //
    //            if (!rdx) continue;
    //
    //            rd = force_get(rdx);
    //
    //            tos::println(*uart, rd);
    //
    //            auto wi = uint8_t(rd[0] - '0');
    //            tos::println(*uart, "partition:", wi);
    //
    //            prog_index = wi;
    //
    //            fetch = ptr_fetcher{prog};
    //            wait.up();
    //
    //            s_wait.down();
    //            for (auto& reg : state.registers)
    //            {
    //                tos::print(*str, reg, " ");
    //            }
    //            tos::println(*str, "");
    //        }
    //        else if (rd[0] == 'p')
    //        {
    //            tos::println(*uart, "program", rd);
    //            tos::println(*str, "send");
    //            rdx = str->read(buffer);
    //
    //            if (!rdx) continue;
    //
    //            rd = force_get(rdx);
    //            tos::println(*uart, rd);
    //
    //            auto wi = uint8_t(rd[0] - '0');
    //
    //            rdx = str->read(buffer);
    //
    //            if (!rdx) continue;
    //
    //            rd = force_get(rdx);
    //
    //            if (uint8_t (rd[0]) > 128)
    //            {
    //                tos::println(*str, "too large");
    //                continue;
    //            }
    //
    //            tos::println(*str, "ok");
    //            str->read({ (char*)prog, rd[0] });
    //            prog_index = wi;
    //            tos::println(*str, "okay");
    //        }
    //        str.reset();
    //    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, tvm_task);
    tos::launch(tos::alloc_stack, main_task);
}
