//
// Created by fatih on 12/26/19.
//

#pragma once

#include "errors.hpp"
#include "fwd.hpp"

#include <common/inet/tcp_ip.hpp>
#include <tos/expected.hpp>
#include <tos/intrusive_list.hpp>

namespace tos::cc32xx {
template <class SocketT>
struct socket_base : list_node<SocketT> {
public:
    explicit socket_base(int16_t handle);

    expected<void, network_errors> bind(tos::port_num_t port);

    int16_t native_handle() {
        return m_handle;
    }

    expected<void, network_errors> set_nonblocking(bool non_blocking);

    expected<void, network_errors> close();

    ~socket_base();

private:

    friend bool operator==(const socket_base& a, const socket_base& b) {
        return a.m_handle == b.m_handle;
    }

    SocketT& self();

    const SocketT& self() const;

    int16_t m_handle;
};

extern template struct socket_base<udp_socket>;
extern template struct socket_base<tcp_socket>;
extern template struct socket_base<tcp_listener>;
} // namespace tos::cc32xx