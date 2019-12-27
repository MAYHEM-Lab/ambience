//
// Created by fatih on 12/26/19.
//

#include <arch/detail/sock_rt.hpp>
#include <arch/detail/socket_base.hpp>
#include <arch/udp.hpp>
#include <arch/tcp.hpp>
#include <ti/drivers/net/wifi/simplelink.h>

namespace tos::cc32xx {
template<class SocketT>
socket_base<SocketT>::socket_base(int16_t handle)
    : m_handle{handle} {
    socket_runtime::instance().register_socket(self());
}

template<class SocketT>
expected<void, network_errors> socket_base<SocketT>::bind(tos::port_num_t port) {
    SlSockAddrIn_t Addr;
    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(port.port);
    Addr.sin_addr.s_addr = 0;

    int16_t AddrSize = sizeof(SlSockAddrIn_t);

    auto res = sl_Bind(m_handle, (SlSockAddr_t*)&Addr, AddrSize);
    if (res != SL_RET_CODE_OK) {
        return unexpected(network_errors(res));
    }

    return {};
}

template<class SocketT>
expected<void, network_errors> socket_base<SocketT>::set_nonblocking(bool non_blocking) {
    SlSockNonblocking_t enableOption;
    enableOption.NonBlockingEnabled = non_blocking;
    auto res = sl_SetSockOpt(native_handle(),
                             SL_SOL_SOCKET,
                             SL_SO_NONBLOCKING,
                             (_u8*)&enableOption,
                             sizeof(enableOption));
    if (res != SL_RET_CODE_OK) {
        return unexpected(network_errors(res));
    }
    return {};
}

template<class SocketT>
socket_base<SocketT>::~socket_base() {
    socket_runtime::instance().remove_socket(self());
    sl_Close(native_handle());
}

template struct socket_base<udp_socket>;
template struct socket_base<tcp_socket>;
template struct socket_base<tcp_listener>;

template<class SocketT>
SocketT& socket_base<SocketT>::self() {
    return *static_cast<SocketT*>(this);
}

template<class SocketT>
const SocketT& socket_base<SocketT>::self() const {
    return *static_cast<const SocketT*>(this);
}
} // namespace tos::cc32xx