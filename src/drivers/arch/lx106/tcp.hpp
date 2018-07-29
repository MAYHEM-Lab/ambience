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
#include <tos/expected.hpp>

#ifdef TOS_HAVE_SSL
#include <compat/lwipr_compat.h>
#include <ssl/tls1.h>

#undef putc
#undef getc
#endif

namespace tos
{
    namespace lwip
    {
        namespace events
        {
            static struct recv_t{} recv{};
            static struct sent_t{} sent{};
            static struct discon_t{} discon{};
        }
    }

    namespace esp82
    {
        enum class connect_error
        {
            not_connected,
            cert_invalid,
            ssl_error,
            unknown
        };

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
            friend err_t accept_handler(void *user, tcp_pcb *new_conn, err_t err);

            friend struct ep_handlers;

            friend expected<tcp_endpoint, connect_error> connect(wifi_connection&, ipv4_addr, port_num_t);

            tcp_pcb* m_conn;
            void* m_event_handler;
        };

        class secure_tcp_endpoint
        {
        public:
            secure_tcp_endpoint() = delete;

            secure_tcp_endpoint(secure_tcp_endpoint&& rhs) noexcept;
            secure_tcp_endpoint(const secure_tcp_endpoint&) = delete;

            template <class EventHandlerT>
            void attach(EventHandlerT& cb);

            void send(span<const char>);

            ~secure_tcp_endpoint();

        private:

            explicit secure_tcp_endpoint(tcp_pcb* conn, SSL* obj, SSLCTX* ctx);
            friend class tcp_socket;

            friend struct sec_ep_handlers;

            friend expected<secure_tcp_endpoint, connect_error> connect_ssl(wifi_connection&, ipv4_addr, port_num_t);

            tcp_pcb* m_conn;
            void* m_event_handler;
            SSL* ssl_obj;
            SSLCTX* ssl_ctx;
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
            friend err_t accept_handler(void *user, tcp_pcb *new_conn, err_t err);
        };

        /**
         * Initiates a TCP connection with the given endpoint.
         *
         * @param iface the interface to connect through
         * @param host ip address of the destination host
         * @param port tcp port of the destination application
         * @return an endpoint or an error if connection fails
         */
        expected<tcp_endpoint, connect_error> connect(wifi_connection& iface, ipv4_addr host, port_num_t port);
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
        err_t accept_handler(void *user, tcp_pcb *new_conn, err_t err)
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
            tcp_accept(m_conn, &accept_handler<ConnHandlerT>);
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

        inline void tcp_endpoint::send(tos::span<const char> buf) {
            tcp_write(m_conn, (uint8_t*)buf.data(), buf.size(), 0);
            tcp_output(m_conn);
        }

        struct ep_handlers
        {
            template <class CallbackT>
            static err_t recv_handler(void* user, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
            {
                auto self = static_cast<tcp_endpoint*>(user);
                auto& handler = *(CallbackT*)self->m_event_handler;
                if (p)
                {
                    handler(lwip::events::recv, *self, span<const char>{ (char*)p->payload, p->len });
                    tcp_recved(tpcb, p->len);
                    pbuf_free(p);
                } else {
                    // conn closed
                    handler(lwip::events::discon, *self);
                    return ERR_OK;
                }
                system_os_post(tos::esp82::main_task_prio, 0, 0);
                return ERR_OK;
            }

            template <class CallbackT>
            static err_t sent_handler(void* user, struct tcp_pcb *tpcb, u16_t)
            {
                auto self = static_cast<tcp_endpoint*>(user);
                self->m_conn = tpcb;

                auto& handler = *(CallbackT*)self->m_event_handler;
                handler(lwip::events::sent, *self);
                system_os_post(tos::esp82::main_task_prio, 0, 0);

                return ERR_OK;
            }

            template <class EventHandlerT>
            static void err_handler(void *user, err_t err)
            {
                auto self = static_cast<tcp_endpoint*>(user);

                auto& handler = *(EventHandlerT*)self->m_event_handler;
                handler(lwip::events::discon, *self);
                system_os_post(tos::esp82::main_task_prio, 0, 0);
            }
        };

        template <class EventHandlerT>
        void tcp_endpoint::attach(EventHandlerT &cb) {
            m_event_handler = &cb;
            tcp_arg(m_conn, this);
            tcp_sent(m_conn, &ep_handlers::sent_handler<EventHandlerT>);
            tcp_recv(m_conn, &ep_handlers::recv_handler<EventHandlerT>);
            tcp_err(m_conn, &ep_handlers::err_handler<EventHandlerT>);
        }

        inline secure_tcp_endpoint::secure_tcp_endpoint(tcp_pcb *conn, SSL* obj, SSLCTX* ctx)
                : m_conn{conn}, ssl_obj{obj}, ssl_ctx{ctx} {
            tcp_arg(m_conn, this);
        }

        inline secure_tcp_endpoint::secure_tcp_endpoint(tos::esp82::secure_tcp_endpoint &&rhs) noexcept
                : m_conn{rhs.m_conn},
                  ssl_obj{rhs.ssl_obj},
                  ssl_ctx{rhs.ssl_ctx},
                  m_event_handler{rhs.m_event_handler}
        {
            tcp_arg(m_conn, this);
            rhs.m_event_handler = nullptr;
            rhs.m_conn = nullptr;
            rhs.ssl_ctx = nullptr;
            rhs.ssl_obj = nullptr;
        }

        inline secure_tcp_endpoint::~secure_tcp_endpoint() {
            if (!m_conn) return;
            tcp_recv(m_conn, nullptr);
            tcp_close(m_conn);
            tcp_abort(m_conn);
        }

        inline void secure_tcp_endpoint::send(tos::span<const char> buf) {
            axl_ssl_write(ssl_obj, (const uint8_t*)buf.data(), buf.size());
            //tcp_write(m_conn, (uint8_t*)buf.data(), buf.size(), 0);
            //tcp_output(m_conn);
        }

        struct sec_ep_handlers
        {
            template <class CallbackT>
            static err_t recv_handler(void* user, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
            {
                auto self = static_cast<secure_tcp_endpoint*>(user);
                auto& handler = *(CallbackT*)self->m_event_handler;
                if (p)
                {
                    pbuf* pout;
                    auto read_bytes = axl_ssl_read(self->ssl_obj, tpcb, p, &pout);

                    if (read_bytes <= 0)
                    {
                        printf("something is wrong");
                    }
                    if (read_bytes > 0)
                    {
                        handler(lwip::events::recv, *self, span<const char>{ (char*)pout->payload, pout->len });
                        pbuf_free(pout);
                    }

                    tcp_recved(tpcb, p->tot_len);
                    pbuf_free(p);
                } else {
                    // conn closed
                    handler(lwip::events::discon, *self);
                    return ERR_OK;
                }
                system_os_post(tos::esp82::main_task_prio, 0, 0);
                return ERR_OK;
            }

            template <class CallbackT>
            static err_t sent_handler(void* user, struct tcp_pcb *tpcb, u16_t)
            {
                auto self = static_cast<secure_tcp_endpoint*>(user);

                auto& handler = *(CallbackT*)self->m_event_handler;
                handler(lwip::events::sent, *self);
                system_os_post(tos::esp82::main_task_prio, 0, 0);

                return ERR_OK;
            }

            template <class EventHandlerT>
            static void err_handler(void *user, err_t err)
            {
                auto self = static_cast<secure_tcp_endpoint*>(user);

                auto& handler = *(EventHandlerT*)self->m_event_handler;
                handler(lwip::events::discon, *self);
                system_os_post(tos::esp82::main_task_prio, 0, 0);
            }
        };

        template <class EventHandlerT>
        void secure_tcp_endpoint::attach(EventHandlerT &cb) {
            m_event_handler = &cb;
            tcp_arg(m_conn, this);
            tcp_sent(m_conn, &sec_ep_handlers::sent_handler<EventHandlerT>);
            tcp_recv(m_conn, &sec_ep_handlers::recv_handler<EventHandlerT>);
            tcp_err(m_conn, &sec_ep_handlers::err_handler<EventHandlerT>);
        }

        struct conn_state
        {
            tos::semaphore sem{0};
            err_t res;
        };

        inline auto connected_handler(void *arg, struct tcp_pcb *tpcb, err_t err) -> err_t
        {
            auto state = static_cast<conn_state*>(arg);
            state->res = err;
            state->sem.up();
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            return ERR_OK;
        }

#ifdef TOS_HAVE_SSL

        SSL *sslObj = NULL;
        SSLCTX* sslContext = NULL;
        int clientfd;

        inline auto ssl_connected_handler(void *arg, struct tcp_pcb *tpcb, err_t err) -> err_t
        {
            auto state = static_cast<conn_state*>(arg);

            clientfd = axl_append(tpcb);

            state->res = err;
            state->sem.up();
            system_os_post(tos::esp82::main_task_prio, 0, 0);
            return ERR_OK;
        }
#endif

        inline void conn_err_handler(void* arg, err_t err)
        {
            auto state = static_cast<conn_state*>(arg);
            state->res = err;
            state->sem.up();
            system_os_post(tos::esp82::main_task_prio, 0, 0);
        }

        inline expected<tcp_endpoint, connect_error> connect(wifi_connection&, ipv4_addr host, port_num_t port) {
            auto pcb = tcp_new();
            ip_addr_t a;
            memcpy(&a.addr, host.addr, 4);
            conn_state state;
            tcp_arg(pcb, &state);
            tcp_connect(pcb, &a, port.port, connected_handler); // this signals ERR_OK
            tcp_err(pcb, conn_err_handler); // this signals ERR_CONN
            state.sem.down();
            if (state.res != ERR_OK)
            {
                return unexpected(connect_error::unknown);
            }
            return tcp_endpoint(pcb);
        }

        inline err_t staticOnReceive(void *arg, struct tcp_pcb *tcp, struct pbuf *p, err_t err)
        {
            struct pbuf* pout;
            int read_bytes = 0;

            printf("Err: %d\n", err);

            if(tcp == NULL || p == NULL) {
                /* @TODO: Take care to handle error conditions */
                printf("Wtf");
                return -1;
            }

            system_soft_wdt_feed();
            read_bytes = axl_ssl_read(sslObj, tcp, p, &pout);
            printf("read: %d", read_bytes);
            tcp_recved(tcp, p->tot_len);
            if(p != NULL) {
                pbuf_free(p);
            }

            if(read_bytes > 0) {
                // free the SSL pbuf and put the decrypted data in the brand new pout pbuf

                printf("Got decrypted data length: %d", read_bytes);

                // put the decrypted data in a brand new pbuf
                p = pout;

                // @TODO: Continue to work with the p buf containing the decrypted data
            }

            return ERR_OK;
        }

        inline expected<secure_tcp_endpoint, connect_error> connect_ssl(wifi_connection&, ipv4_addr host, port_num_t port) {
            auto pcb = tcp_new();
            ip_addr_t a;
            memcpy(&a.addr, host.addr, 4);
            conn_state state;
            tcp_arg(pcb, &state);
            tcp_recv(pcb, staticOnReceive);
            tcp_err(pcb, conn_err_handler); // this signals ERR_CONN

            tcp_connect(pcb, &a, port.port, ssl_connected_handler); // this signals ERR_OK
            state.sem.down();

            sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | SSL_SERVER_VERIFY_LATER | SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES, 1);
            auto ext = ssl_ext_new();
            ext->host_name = "bakirbros.com";
            system_update_cpu_freq(SYS_CPU_160MHZ);
            sslObj = ssl_client_new(sslContext, clientfd, nullptr, 0, ext);

            while (ssl_handshake_status(sslObj) != SSL_OK)
            {
                tos::this_thread::yield();
            }
            system_update_cpu_freq(SYS_CPU_80MHZ);

            if (state.res != ERR_OK)
            {
                return unexpected(connect_error::unknown);
            }
            return secure_tcp_endpoint(pcb, sslObj, sslContext);
        }
    }
}