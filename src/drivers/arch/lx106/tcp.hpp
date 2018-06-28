//
// Created by fatih on 6/28/18.
//

#pragma once

#include <tos/intrusive_list.hpp>
#include <wifi.hpp>

extern "C"
{
#include <user_interface.h>
#include <espconn.h>
}

namespace tos
{
    struct port_num_t
    {
        uint16_t port;
    };

    namespace esp82
    {
        class tcp_socket
                : public tos::list_node<tcp_socket>
        {
        public:
            explicit tcp_socket(tos::esp82::wifi&, port_num_t port);

            template <class ConnHandlerT>
            void accept(ConnHandlerT& handler);

            static tcp_socket* find_sock(int port);
        private:
            espconn m_conn{};
            esp_tcp m_tcp{};
            void* m_accept_handler = nullptr;

            template <class ConnHandlerT>
            friend void conn_handler(void* arg);
        };

    }
}

namespace tos
{
    namespace esp82
    {
        template <class ConnHandlerT>
        void conn_handler(void* arg)
        {
            auto new_conn = static_cast<espconn*>(arg);
            auto self = tcp_socket::find_sock(new_conn->proto.tcp->local_port);
            auto handler = *(ConnHandlerT*)self->m_accept_handler;
            handler(new_conn);
        }

        template <class ConnHandlerT>
        void tcp_socket::accept(ConnHandlerT &handler) {
            m_accept_handler = &handler;

            espconn_regist_connectcb(&m_conn, &conn_handler<ConnHandlerT>);
            espconn_accept(&m_conn);
        }
    }
}