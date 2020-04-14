//
// Created by fatih on 12/26/19.
//

#include <arch/udp.hpp>
#include <ti/drivers/net/wifi/simplelink.h>

namespace tos::cc32xx {
udp_socket::udp_socket()
    : socket_base(sl_Socket(SL_AF_INET, SL_SOCK_DGRAM, SL_IPPROTO_UDP)) {
    socket_base::set_nonblocking(true);
}

expected<void, network_errors> udp_socket::send_to(tos::span<const uint8_t> data,
                                   const tos::udp_endpoint_t& to) {
    SlSockAddrIn_t Addr;
    Addr.sin_family = SL_AF_INET;
    Addr.sin_port = sl_Htons(to.port.port);
    Addr.sin_addr.s_addr = sl_Htonl(SL_IPV4_VAL(
                                        to.addr.addr[0], to.addr.addr[1], to.addr.addr[2], to.addr.addr[3]));

    int16_t AddrSize = sizeof(SlSockAddrIn_t);

    auto result = sl_SendTo(native_handle(),
                            data.data(),
                            data.size_bytes(),
                            0,
                            (SlSockAddr_t*)&Addr,
                            AddrSize);

    if (result < 0) {
        return unexpected(network_errors(result));
    }

    return {};
}

expected<span<uint8_t>, network_errors> udp_socket::receive_from(tos::span<uint8_t> data,
                                                        tos::udp_endpoint_t& from) {
    m_receive_sem.down();

    SlSockAddrIn_t Addr;
    uint16_t addr_len = sizeof Addr;
    auto ret = sl_RecvFrom(native_handle(),
                           data.data(),
                           data.size(),
                           0,
                           (SlSockAddr_t*)&Addr,
                           &addr_len);
    if (ret < 0) {
        return unexpected(network_errors(ret));
    }
    return data.slice(0, ret);
}
}