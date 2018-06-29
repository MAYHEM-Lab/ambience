//
// Created by fatih on 6/28/18.
//

#pragma once

#include <tos/intrusive_list.hpp>
#include <wifi.hpp>
#include <tos/arch.hpp>
#include <utility>
#include <algorithm>

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
        namespace events
        {
            static struct recv_t{} recv{};
            static struct sent_t{} sent{};
        }

        class tcp_endpoint
        {
        public:
            tcp_endpoint(tcp_endpoint&& rhs) noexcept;
            tcp_endpoint(const tcp_endpoint&) = delete;

            template <class CallbackT>
            void attach(events::recv_t, CallbackT& cb);

            template <class CallbackT>
            void attach(events::sent_t, CallbackT& cb);

            void send(span<const char>);

            ~tcp_endpoint();

        private:

            explicit tcp_endpoint(espconn* conn);
            friend class tcp_socket;

            template <class ConnHandlerT>
            friend void conn_handler(void* arg);

            template <class CallbackT>
            friend void recv_handler(void* arg, char *pdata, unsigned short len);

            template <class ConnHandlerT>
            friend void sent_handler(void* arg);

            espconn* m_conn;
            void* m_recv_handler;
            void* m_sent_handler;
        };

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
            handler(*self, tcp_endpoint{ new_conn });
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class ConnHandlerT>
        void tcp_socket::accept(ConnHandlerT &handler) {
            m_accept_handler = &handler;

            espconn_regist_connectcb(&m_conn, &conn_handler<ConnHandlerT>);
            espconn_accept(&m_conn);
        }

        inline tcp_endpoint::tcp_endpoint(espconn *conn) : m_conn{conn} {
        }

        inline tcp_endpoint::tcp_endpoint(tos::esp82::tcp_endpoint &&rhs) noexcept
                : m_conn{rhs.m_conn}, m_recv_handler{rhs.m_recv_handler},
                m_sent_handler{rhs.m_sent_handler}
        {
            m_conn->reverse = this;
            rhs.m_sent_handler = nullptr;
            rhs.m_recv_handler = nullptr;
            rhs.m_conn = nullptr;
        }

        inline tcp_endpoint::~tcp_endpoint() {
            espconn_delete(m_conn);
        }

        template <class CallbackT>
        void recv_handler(void* arg, char *pdata, unsigned short len)
        {
            auto conn = static_cast<espconn*>(arg);
            auto self = static_cast<tcp_endpoint*>(conn->reverse);

            auto handler = *(CallbackT*)self->m_recv_handler;
            handler(*self, span<const char>{pdata, len});
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class CallbackT>
        void sent_handler(void* arg)
        {
            auto conn = static_cast<espconn*>(arg);
            auto self = static_cast<tcp_endpoint*>(conn->reverse);

            auto handler = *(CallbackT*)self->m_sent_handler;
            handler(*self);
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class CallbackT>
        void tcp_endpoint::attach(events::recv_t, CallbackT &cb) {
            m_conn->reverse = this;
            m_recv_handler = &cb;
            espconn_regist_recvcb(m_conn, &recv_handler<CallbackT>);
        }

        template <class CallbackT>
        void tcp_endpoint::attach(events::sent_t, CallbackT &cb) {
            m_conn->reverse = this;
            m_sent_handler = &cb;
            espconn_regist_sentcb(m_conn, &sent_handler<CallbackT>);
        }

        inline void tcp_endpoint::send(tos::span<const char> buf) {
            espconn_send(m_conn, (uint8_t*)buf.data(), buf.size());
        }
    }
}