//
// Created by fatih on 12/26/19.
//

#pragma once

#include "detail/socket_base.hpp"

#include <tos/fixed_fifo.hpp>
#include <tos/function_ref.hpp>
#include <tos/semaphore.hpp>
#include <tos/sync_ring_buf.hpp>
#include <tos/intrusive_ptr.hpp>

namespace tos::cc32xx {
class tcp_socket : public socket_base<tcp_socket> {
public:
    explicit tcp_socket(int16_t handle);

    void signal_select_rx();

    void signal_select_tx() {
    }

    expected<span<uint8_t>, network_errors> read(span<uint8_t> buffer);

    expected<size_t, network_errors> write(span<const uint8_t> buffer);

    bool disconnected() const {
        return m_closed;
    }

private:
    bool m_closed = false;
    semaphore_base<int16_t> m_len{0};
    basic_fixed_fifo<uint8_t, 512, ring_buf> m_recv_buffer;
};

class tcp_listener : public socket_base<tcp_listener> {
public:
    explicit tcp_listener(tos::port_num_t port);

    expected<void, network_errors> listen();

    expected<intrusive_ptr<tcp_socket>, network_errors> accept();

    using socket_base::native_handle;

    void signal_select();

private:
    semaphore m_accept_sem{0};
};

enum class connect_errors
{
    socket_error,
    connect_error
};

expected<intrusive_ptr<tcp_socket>, connect_errors>
connect(class simplelink_wifi& wifi, ipv4_addr_t addr, port_num_t port);
} // namespace tos::cc32xx
