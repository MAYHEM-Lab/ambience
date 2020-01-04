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
    m_accept_sem.up();
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
    auto sock = new (std::nothrow) tcp_socket(accept_res);
    if (!sock) {
        return unexpected(network_errors(SL_ERROR_UTILS_MEM_ALLOC));
    }
    tos::println(log, "Got socket:", int(accept_res));
    return sock;
}

tcp_socket::tcp_socket(int16_t handle)
    : socket_base(handle) {
    socket_base::set_nonblocking(true);
}

void tcp_socket::signal_select_rx() {
    while (true) {
        // Read in 16 byte chunks.
        // TODO(fatih): use the receive buffer directly for better performance.
        std::array<uint8_t, 16> buf;
        auto res = sl_Recv(native_handle(), buf.data(), buf.size(), 0);
        if (res < 0) {
            return;
        }
        if (res == 0) {
            tos::println(log, "received 0 bytes, end of stream!");
            close();
            return;
        }
        auto s = span(buf).slice(0, res);
        //tos::println(log, "received:", raw_cast<const char>(s), int(res));
        for (auto byte : buf) {
            if (m_recv_buffer.size() == m_recv_buffer.capacity()) {
                // Out of buffer space
                return;
            }
            m_recv_buffer.push(byte);
            m_len.up();
        }
    }
}

expected<span<uint8_t>, network_errors> tcp_socket::read(span<uint8_t> buffer) {
    auto tmp_buffer = buffer;
    while (!tmp_buffer.empty()) {
        m_len.down();
        tmp_buffer.front() = m_recv_buffer.pop();
        tmp_buffer = tmp_buffer.slice(1);
    }
    return buffer;
}

expected<size_t, network_errors> tcp_socket::write(span<const uint8_t> buffer) {
    auto res = sl_Send(native_handle(), buffer.data(), buffer.size(), 0);
    while (res == SL_ERROR_BSD_EAGAIN) {
        tos::this_thread::yield();
        res = sl_Send(native_handle(), buffer.data(), buffer.size(), 0);
    }
    return buffer.size();
}
} // namespace tos::cc32xx