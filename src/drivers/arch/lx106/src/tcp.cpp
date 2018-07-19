//
// Created by fatih on 6/28/18.
//

#include <tcp.hpp>
#include <tos/print.hpp>

static tos::intrusive_list<tos::esp82::tcp_socket> sks;

static tos::track_ptr<tos::esp82::tcp_endpoint> eps[5];

namespace tos
{
    namespace esp82
    {


        tcp_socket* tcp_socket::find_sock(int port)
        {
            for (auto& sock : sks)
            {
                if (sock.m_tcp.local_port == port)
                {
                    return &sock;
                }
            }
            return nullptr;
        }

        void detail::new_endpoint(tcp_endpoint &new_ep) {
            for (auto& ep : eps)
            {
                if (ep == nullptr)
                {
                    ep = get_ptr(new_ep);
                    return;
                }
            }
        }

        tcp_endpoint *detail::find_endpoint(const ipv4_addr & addr, port_num_t port) {
            for (auto& ep : eps)
            {
                if (ep && ep->get_remote_port() == port && ep->get_remote_addr() == addr)
                {
                    return ep.get();
                }
            }
            return nullptr;
        }

        void prnt_debug(esp82::uart0 &uart) {
            for (auto& ep : eps)
            {
                tos::print(uart, ep.get(), "");
                if (ep)
                {
                    tos::print(uart, int(ep->get_remote_port().port));
                }
                tos::println(uart);
            }
        }
    }
}
