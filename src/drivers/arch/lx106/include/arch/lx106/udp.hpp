//
// Created by fatih on 2/22/19.
//

#pragma once

#include <tos/span.hpp>
#include <common/inet/tcp_ip.hpp>
#include <lwip/udp.h>

namespace tos
{
namespace esp
{
    struct udp_endpoint_t
    {
        ipv4_addr_t addr;
        port_num_t port;
    };

    template <class CbT>
    void udp_receiver(void* arg, udp_pcb* pcb, pbuf* pbuf, ip_addr_t* addr, u16 port);

    class async_udp_socket
    {
    public:
        void send_to(tos::span<const uint8_t> buf, udp_endpoint_t to)
        {
        }

        void recv_from(tos::span<uint8_t> buf){
            udp_recv(m_pcb, m_recv, m_handler);
        }

        template <class EvHandlerT>
        void attach(EvHandlerT& handler)
        {
            m_handler = &handler;
            m_recv = &udp_receiver<EvHandlerT>;
        }

    private:

        udp_pcb* m_pcb;
        udp_recv_fn m_recv;
        void* m_handler;
    };

    class udp_socket
    {
    public:

        void send_to(tos::span<const uint8_t>, ipv4_addr_t addr, port_num_t port);
    private:
    };
}
}
