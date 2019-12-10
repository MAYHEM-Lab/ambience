//
// Created by fatih on 2/22/19.
//

#pragma once

#include <lwip/udp.h>
#include <cstring>
#include <common/inet/lwip.hpp>
#include <common/inet/tcp_ip.hpp>
#include <tos/expected.hpp>
#include <tos/span.hpp>

namespace tos {
namespace esp82 {

template<class CbT>
void udp_receiver(void* arg, udp_pcb* pcb, pbuf* pbuf, ip_addr_t* addr, u16 port);

class async_udp_socket {
public:
    async_udp_socket() {
        m_pcb = udp_new();
        m_handler = nullptr;
    }

    async_udp_socket(async_udp_socket&&) = delete;

    expected<void, err_t> bind(port_num_t port) {
        auto err = udp_bind(m_pcb, IP_ADDR_ANY, port.port);
        if (err != ERR_OK) {
            return unexpected(err);
        }
        return {};
    }

    expected<void, err_t> send_to(tos::span<const uint8_t> buf,
                                  const udp_endpoint_t& to) {
        auto pbuf = pbuf_alloc(PBUF_TRANSPORT, buf.size(), PBUF_RAM);

        if (!pbuf) {
            return unexpected(ERR_MEM);
        }

        memcpy(pbuf->payload, buf.data(), buf.size());

        auto err = udp_sendto(
            m_pcb,
            pbuf,
            reinterpret_cast<ip_addr_t*>(const_cast<uint8_t*>(&to.addr.addr[0])),
            to.port.port);

        pbuf_free(pbuf);

        if (err != ERR_OK) {
            return unexpected(err);
        }

        return {};
    }

    template<class EvHandlerT>
    void attach(EvHandlerT& handler) {
        m_handler = &handler;
        udp_recv(m_pcb, &udp_receiver<EvHandlerT>, this);
    }

    ~async_udp_socket() {
        if (!m_pcb)
            return;

        udp_recv(m_pcb, nullptr, nullptr);
        udp_disconnect(m_pcb);
        udp_remove(m_pcb);
    }

private:
    template<class CbT>
    friend void
    udp_receiver(void* arg, udp_pcb* pcb, pbuf* pbuf, ip_addr_t* addr, u16 port);

    udp_pcb* m_pcb;
    void* m_handler;
};

template<class CbT>
void udp_receiver(void* arg, udp_pcb* pcb, pbuf* pbuf, ip_addr_t* addr, u16 port) {
    auto self = static_cast<async_udp_socket*>(arg);
    auto& handler = *static_cast<CbT*>(self->m_handler);

    udp_endpoint_t ep;
    ep.port.port = port;
    std::memcpy(ep.addr.addr, addr, 4);

    handler(lwip::events::recvfrom, self, ep, lwip::buffer{pbuf});
}
} // namespace esp82
} // namespace tos
