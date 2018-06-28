//
// Created by fatih on 6/28/18.
//

#include <tcp.hpp>

static tos::intrusive_list<tos::esp82::tcp_socket> sks;

namespace tos
{
    namespace esp82
    {
        tcp_socket::tcp_socket(tos::esp82::wifi&, port_num_t port) {
            m_conn.type = ESPCONN_TCP;
            m_conn.state = ESPCONN_NONE;
            m_conn.reverse = this;

            m_tcp.local_port = port.port;
            m_conn.proto.tcp = &m_tcp;
            sks.push_back(*this);
        }

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
    }
}
