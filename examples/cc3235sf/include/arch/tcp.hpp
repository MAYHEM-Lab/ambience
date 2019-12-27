//
// Created by fatih on 12/26/19.
//

#pragma once

#include "detail/socket_base.hpp"

#include <tos/function_ref.hpp>
#include <tos/semaphore.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/sync_ring_buf.hpp>

namespace tos::cc32xx {
class tcp_socket : public socket_base<tcp_socket> {
public:
    explicit tcp_socket(int16_t handle);

    void signal_select_rx();

    void signal_select_tx() {
    }

    expected<span<uint8_t>, network_errors> receive(span<uint8_t> buffer);

private:
    sync_fixed_fifo<uint8_t, 64> buffer;
};

class tcp_listener : public socket_base<tcp_listener> {
public:
    explicit tcp_listener(tos::port_num_t port);

    expected<void, network_errors> listen();

    expected<tcp_socket*, network_errors> accept();

    using socket_base::native_handle;

    void signal_select();

private:
    semaphore m_accept_sem{0};
};
}
