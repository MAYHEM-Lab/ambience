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
#include <lwip/tcp.h>

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

        class tcp_endpoint
        {
        public:
            tcp_endpoint() = delete;

            tcp_endpoint(tcp_endpoint&& rhs) noexcept;
            tcp_endpoint(const tcp_endpoint&) = delete;

            template <class EventHandlerT>
            void attach(EventHandlerT& cb);

            void send(span<const char>);

            ~tcp_endpoint();

        private:

            explicit tcp_endpoint(tcp_pcb* conn);
            friend class tcp_socket;

            template <class ConnHandlerT>
            friend err_t conn_handler(void* user, tcp_pcb* new_conn, err_t err);

            template <class CallbackT>
            friend err_t recv_handler(void* user, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

            template <class ConnHandlerT>
            friend err_t sent_handler(void* user, struct tcp_pcb *tpcb, u16_t len);

            template <class EventHandlerT>
            friend void err_handler(void *user, err_t err);

            tcp_pcb* m_conn;
            void* m_event_handler;
        };

        class tcp_socket
                : public tos::list_node<tcp_socket>
        {
        public:
            explicit tcp_socket(tos::esp82::wifi&, port_num_t port);
            ~tcp_socket();

            template <class ConnHandlerT>
            void accept(ConnHandlerT& handler);

            bool is_valid() const { return m_conn; }

        private:
            tcp_pcb* m_conn = nullptr;
            void* m_accept_handler = nullptr;

            template <class ConnHandlerT>
            friend err_t conn_handler(void* user, tcp_pcb* new_conn, err_t err);
        };
    }
}

namespace tos
{
    namespace esp82
    {
        inline tcp_socket::tcp_socket(tos::esp82::wifi&, port_num_t port) {
            auto pcb = tcp_new();

            auto err = tcp_bind(pcb, IP_ADDR_ANY, port.port);

            if (err != ERR_OK) {
                tcp_close(pcb);
                return;
            }

            auto listen_pcb = tcp_listen(pcb);
            if (!listen_pcb) {
                tcp_close(pcb);
                return;
            }

            m_conn = listen_pcb;
        }

        template <class ConnHandlerT>
        err_t conn_handler(void* user, tcp_pcb* new_conn, err_t err)
        {
            if (err != ERR_OK)
            {
                return ERR_OK;
            }
            auto self = static_cast<tcp_socket*>(user);

            auto& handler = *(ConnHandlerT*)self->m_accept_handler;
            auto res = handler(*self, tcp_endpoint{ new_conn });
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            return ERR_OK;
        }

        template <class ConnHandlerT>
        void tcp_socket::accept(ConnHandlerT &handler) {

            m_accept_handler = &handler;

            tcp_arg(m_conn, this);
            tcp_accept(m_conn, &conn_handler<ConnHandlerT>);
        }

        inline tcp_socket::~tcp_socket() {
            tcp_close(m_conn);
        }

        inline tcp_endpoint::tcp_endpoint(tcp_pcb *conn)
                : m_conn{conn} {
            tcp_arg(m_conn, this);
        }

        inline tcp_endpoint::tcp_endpoint(tos::esp82::tcp_endpoint &&rhs) noexcept
                : m_conn{rhs.m_conn},
                  m_event_handler{rhs.m_event_handler}
        {
            tcp_arg(m_conn, this);
            rhs.m_event_handler = nullptr;
            rhs.m_conn = nullptr;
        }

        inline tcp_endpoint::~tcp_endpoint() {
            if (!m_conn) return;
            tcp_recv(m_conn, nullptr);
            tcp_close(m_conn);
            tcp_abort(m_conn);
        }

        template <class CallbackT>
        err_t recv_handler(void* user, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
        {
            auto self = static_cast<tcp_endpoint*>(user);
            self->m_conn = tpcb;

            if (p)
            {
                auto& handler = *(CallbackT*)self->m_event_handler;
                handler(events::recv, *self, span<const char>{ (char*)p->payload, p->len });
                tcp_recved(tpcb, p->len);
                pbuf_free(p);
            } else {
                // conn closed
                auto& handler = *(CallbackT*)self->m_event_handler;
                handler(events::discon, *self);
                return ERR_OK;
            }
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            return ERR_OK;
        }

        template <class CallbackT>
        err_t sent_handler(void* user, struct tcp_pcb *tpcb, u16_t)
        {
            auto self = static_cast<tcp_endpoint*>(user);
            self->m_conn = tpcb;

            auto& handler = *(CallbackT*)self->m_event_handler;
            handler(events::sent, *self);
            system_os_post(tos::esp82::main_task_prio, 0, 0);

            return ERR_OK;
        }

        template <class EventHandlerT>
        void err_handler(void *user, err_t err)
        {
            auto self = static_cast<tcp_endpoint*>(user);

            auto& handler = *(EventHandlerT*)self->m_event_handler;
            handler(events::discon, *self);
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        template <class EventHandlerT>
        void tcp_endpoint::attach(EventHandlerT &cb) {
            m_event_handler = &cb;
            tcp_arg(m_conn, this);
            tcp_sent(m_conn, &sent_handler<EventHandlerT>);
            tcp_recv(m_conn, &recv_handler<EventHandlerT>);
            tcp_err(m_conn, &err_handler<EventHandlerT>);
        }

        inline void tcp_endpoint::send(tos::span<const char> buf) {
            tcp_write(m_conn, (uint8_t*)buf.data(), buf.size(), 0);
            tcp_output(m_conn);
        }
    }
}