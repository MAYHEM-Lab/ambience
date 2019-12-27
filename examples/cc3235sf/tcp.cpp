//
// Created by fatih on 12/27/19.
//

#include <arch/tcp.hpp>
#include <common/usart.hpp>
#include <ti/drivers/net/wifi/simplelink.h>

extern tos::any_usart* log;
namespace tos::cc32xx {
tcp_listener::tcp_listener(port_num_t port)
    : socket_base(sl_Socket(SL_AF_INET, SL_SOCK_STREAM, SL_IPPROTO_TCP)) {
    socket_base::bind(port);
    socket_base::set_nonblocking(true);
}

expected<void, network_errors> tcp_listener::listen() {
    auto res = sl_Listen(native_handle(), 4);
    if (res != SL_RET_CODE_OK) {
        return unexpected(network_errors(res));
    }
    return {};
}

void tcp_listener::signal_select() {
    m_accept_sem.up_isr();
}

expected<tcp_socket*, network_errors> tcp_listener::accept() {
    m_accept_sem.down();
    SlSockAddrIn_t Addr;
    uint16_t addr_len = sizeof Addr;
    auto accept_res = sl_Accept(native_handle(), (SlSockAddr_t*)&Addr, &addr_len);
    if (accept_res < 0) {
        // wtf
        tos::println(log, "bad accept");
        return unexpected(network_errors(accept_res));
    }
    tos::println(log, "Got socket:", int(accept_res));
    return new tcp_socket(accept_res);
}

tcp_socket::tcp_socket(int16_t handle)
    : socket_base(handle) {
    socket_base::set_nonblocking(true);
}

void tcp_socket::signal_select_rx() {
    while (true) {
        std::array<uint8_t, 16> buf;
        auto res = sl_Recv(native_handle(), buf.data(), buf.size(), 0);
        if (res < 0) {
            return;
        }
        auto s = span<const uint8_t>(buf).slice(0, res);
        tos::println(log, "received:", raw_cast<const char>(s));

    }
}
} // namespace tos::cc32xx