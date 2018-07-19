//
// Created by fatih on 6/28/18.
//

#pragma once

#include <tos/intrusive_list.hpp>
#include <wifi.hpp>
#include <tos/arch.hpp>
#include <utility>
#include <algorithm>
#include <common/inet/tcp_ip.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/track_ptr.hpp>
#include "usart.hpp"

extern "C"
{
#include <user_interface.h>
#include <espconn.h>
}

extern tos::function_ref<void(const char*)> print_debug;

namespace tos
{
    namespace esp82
    {
        namespace events
        {
            static struct recv_t{} recv{};
            static struct sent_t{} sent{};
            static struct discon_t{} discon{};
        }

        /**
         * Extracts the remote ip address from the given espconn object.
         *
         * The given espconn object must represent a connected tcp connection.
         *
         * @return remote ip address
         */
        ipv4_addr extract_remote_ip(const espconn&);

        /**
         * Extracts the remote port from the given espconn object.
         *
         * The given espconn object must represent a connected tcp connection.
         *
         * @return remote port number
         */
        port_num_t extract_remote_port(const espconn&);

        class tcp_endpoint : public tracked
        {
        public:
            tcp_endpoint() = delete;

            tcp_endpoint(tcp_endpoint&& rhs) noexcept;
            tcp_endpoint(const tcp_endpoint&) = delete;

            template <class CallbackT>
            void attach(events::recv_t, CallbackT& cb);

            template <class CallbackT>
            void attach(events::sent_t, CallbackT& cb);

            template <class CallbackT>
            void attach(events::discon_t, CallbackT& cb);

            void send(span<const char>);

            ipv4_addr get_remote_addr() const { return m_remote_addr; }
            port_num_t get_remote_port() const { return m_remote_port; }

            ~tcp_endpoint();

        private:

            explicit tcp_endpoint(espconn* conn);
            friend class tcp_socket;

            template <class ConnHandlerT>
            friend void conn_handler(void* arg);

            template <class ConnHandlerT>
            friend void discon_handler(void* arg);

            template <class CallbackT>
            friend void recv_handler(void* arg, char *pdata, unsigned short len);

            template <class ConnHandlerT>
            friend void sent_handler(void* arg);

            espconn* m_conn;
            ipv4_addr m_remote_addr;
            port_num_t m_remote_port;
            void* m_recv_handler;
            void* m_sent_handler;
            void* m_discon_handler;
        };

        void prnt_debug(esp82::uart0& uart);

        namespace detail
        {
            void new_endpoint(tcp_endpoint& ep);
            tcp_endpoint* find_endpoint(const ipv4_addr&, port_num_t);
        }

        class tcp_socket
                : public tos::list_node<tcp_socket>
        {
        public:
            explicit tcp_socket(tos::esp82::wifi&, port_num_t port);
            ~tcp_socket();

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
        inline tcp_socket::tcp_socket(tos::esp82::wifi&, port_num_t port) {
            sks.push_back(*this);
        }

        inline ipv4_addr extract_remote_ip(const espconn & from) {
            ipv4_addr res;
            memcpy(&res.addr, &from.proto.tcp->remote_ip, 4);
            return res;
        }

        inline port_num_t extract_remote_port(const espconn & from) {
            return { from.proto.tcp->remote_port };
        }

        inline void recon_handler(void* arg, sint8_t err)
        {
            auto conn = static_cast<espconn*>(arg);
            print_debug("recon");
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class ConnHandlerT>
        void conn_handler(void* arg)
        {
            auto new_conn = static_cast<espconn*>(arg);
            auto self = tcp_socket::find_sock(new_conn->proto.tcp->local_port);

            auto& handler = *(ConnHandlerT*)self->m_accept_handler;
            handler(*self, tcp_endpoint{ new_conn });
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class ConnHandlerT>
        void tcp_socket::accept(ConnHandlerT &handler) {
            m_accept_handler = &handler;

            espconn_regist_connectcb(&m_conn, &conn_handler<ConnHandlerT>);
            espconn_regist_reconcb(&m_conn, recon_handler);
            espconn_accept(&m_conn);
        }

        inline tcp_socket::~tcp_socket() {
            espconn_delete(&m_conn);
        }


        inline tcp_endpoint::tcp_endpoint(espconn *conn)
                : m_conn{conn} {
            print_debug("create ep");
            m_remote_addr = extract_remote_ip(*conn);
            m_remote_port = extract_remote_port(*conn);
            detail::new_endpoint(*this);
        }

        inline tcp_endpoint::tcp_endpoint(tos::esp82::tcp_endpoint &&rhs) noexcept
                : m_conn{rhs.m_conn},
                  m_remote_addr{rhs.m_remote_addr},
                  m_remote_port{rhs.m_remote_port},
                  m_recv_handler{rhs.m_recv_handler},
                  m_sent_handler{rhs.m_sent_handler},
                  m_discon_handler{rhs.m_discon_handler},
                  tracked(std::move(rhs))
        {
            print_debug("move ep");

            rhs.m_sent_handler = nullptr;
            rhs.m_recv_handler = nullptr;
            rhs.m_discon_handler = nullptr;
            rhs.m_conn = nullptr;
            rhs.m_remote_port = { 0 };
        }

        inline tcp_endpoint::~tcp_endpoint() {
            if (!m_conn) return;
            espconn_disconnect(m_conn);
            espconn_delete(m_conn);
        }

        template <class CallbackT>
        void recv_handler(void* arg, char *pdata, unsigned short len)
        {
            auto conn = static_cast<espconn*>(arg);
            auto self = detail::find_endpoint(extract_remote_ip(*conn), extract_remote_port(*conn));
            self->m_conn = conn;

            auto& handler = *(CallbackT*)self->m_recv_handler;
            handler(events::recv, *self, span<const char>{pdata, len});
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class CallbackT>
        void sent_handler(void* arg)
        {
            auto conn = static_cast<espconn*>(arg);
            auto self = detail::find_endpoint(extract_remote_ip(*conn), extract_remote_port(*conn));
            self->m_conn = conn;

            auto& handler = *(CallbackT*)self->m_sent_handler;
            handler(events::sent, *self);
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class CallbackT>
        void discon_handler(void* arg)
        {
            auto conn = static_cast<espconn*>(arg);
            //auto self = detail::find_endpoint(extract_remote_ip(*conn), extract_remote_port(*conn));
            //self->m_conn = conn;
            print_debug("discon");
            //auto& handler = *(CallbackT*)self->m_discon_handler;
            //handler(events::discon, *self);
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class CallbackT>
        void tcp_endpoint::attach(events::recv_t, CallbackT &cb) {
            m_recv_handler = &cb;
            espconn_regist_recvcb(m_conn, &recv_handler<CallbackT>);
        }

        template <class CallbackT>
        void tcp_endpoint::attach(events::sent_t, CallbackT &cb) {
            m_sent_handler = &cb;
            espconn_regist_sentcb(m_conn, &sent_handler<CallbackT>);
        }

        template <class CallbackT>
        void tcp_endpoint::attach(events::discon_t, CallbackT &cb) {
            m_discon_handler = &cb;
            espconn_regist_disconcb(m_conn, &discon_handler<CallbackT>);
        }

        inline void tcp_endpoint::send(tos::span<const char> buf) {
            espconn_send(m_conn, (uint8_t*)buf.data(), buf.size());
        }
    }
}