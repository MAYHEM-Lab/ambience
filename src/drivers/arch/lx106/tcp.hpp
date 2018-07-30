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
#include <tos/algorithm.hpp>

#ifdef TOS_HAVE_SSL
#include <compat/lwipr_compat.h>
#undef putc
#undef getc
#undef printf
#include <ssl/tls1.h>
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

        enum class discon_reason
        {
            closed,
            ssl_error,
            recv_error,
            aborted,
            reset,
            other_error
        };

        struct buffer
        {
            buffer() : m_root{nullptr}, m_read_off{0} {}
            explicit buffer(pbuf* buf) : m_root{buf}, m_read_off{0} {}

            buffer(const buffer&) = delete;
            buffer(buffer&& rhs) noexcept : m_root{rhs.m_root}, m_read_off{rhs.m_read_off}
            {
                rhs.m_root = nullptr;
            }

            buffer& operator=(buffer&& rhs) noexcept
            {
                if (m_root)
                {
                    pbuf_free(m_root);
                }
                m_root = rhs.m_root;
                m_read_off = rhs.m_read_off;
                rhs.m_root = nullptr;
                return *this;
            }

            ~buffer()
            {
                if (m_root)
                {
                    pbuf_free(m_root);
                }
            }

            span<char> read(span<char> buf)
            {
                tos::int_guard ig;
                auto remaining = tos::std::min(buf.size(), size());

                auto total = 0;
                for (auto out_it = buf.begin(); remaining > 0;)
                {
                    auto bucket = cur_bucket();
                    auto bucket_sz = tos::std::min(bucket.size(), remaining);
                    std::copy(bucket.begin(), bucket.begin() + bucket_sz, out_it);
                    out_it += bucket_sz;
                    total += bucket_sz;
                    remaining -= bucket_sz;
                    consume(bucket_sz);
                }

                return buf.slice(0, total);
            }

            size_t size() const {
                if (!m_root) return 0;
                return m_root->tot_len - m_read_off;
            }

            bool has_more() const
            {
                return m_root;
            }

            void append(buffer&& b)
            {
                auto buf = b.m_root;
                b.m_root = nullptr;
                pbuf_cat(m_root, buf);
            }

        private:

            /**
             * Consumes the given number of bytes from the pbuf chain
             *
             * Number of bytes must be less than or equal to the cur_bucket().size()
             *
             * @param sz number of bytes
             */
            void consume(size_t sz)
            {
                if (sz < cur_bucket().size())
                {
                    m_read_off += sz;
                    return;
                }

                if (!m_root->next)
                {
                    // done
                    pbuf_free(m_root);
                    m_root = nullptr;
                    return;
                }

                auto next = m_root->next;
                pbuf_ref(next);
                pbuf_free(m_root);
                m_root = next;
                m_read_off = 0;
            }

            span<char> cur_bucket()
            {
                return { (char*)m_root->payload + m_read_off, m_root->len - m_read_off};
            }

            span<const char> cur_bucket() const
            {
                return { (const char*)m_root->payload + m_read_off, m_root->len - m_read_off};
            }

            pbuf* m_root;
            size_t m_read_off;
        };
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
            tcp_err(m_conn, nullptr);
            tcp_sent(m_conn, nullptr);
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
                system_os_post(tos::esp82::main_task_prio, 0, 0);
                if (err != ERR_OK)
                {
                    handler(lwip::events::discon, *self, lwip::discon_reason::recv_error);
                    return ERR_OK;
                }
                if (p)
                {
                    tcp_recved(tpcb, p->tot_len);
                    handler(lwip::events::recv, *self, lwip::buffer{ p });
                } else {
                    // conn closed
                    handler(lwip::events::discon, *self, lwip::discon_reason::closed);
                    return ERR_OK;
                }
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
                handler(lwip::events::discon, *self, err == ERR_ABRT ?
                                        lwip::discon_reason::aborted : lwip::discon_reason::reset);
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
            ssl_ctx_free(ssl_ctx);
            axl_free(m_conn);
            tcp_recv(m_conn, nullptr);
            tcp_err(m_conn, nullptr);
            tcp_sent(m_conn, nullptr);
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
                auto ret_code = ERR_OK;
                auto self = static_cast<secure_tcp_endpoint*>(user);
                auto& handler = *(CallbackT*)self->m_event_handler;
                system_os_post(tos::esp82::main_task_prio, 0, 0);

                if (err != ERR_OK)
                {
                    handler(lwip::events::discon, *self, lwip::discon_reason::recv_error);
                    return ERR_ABRT;
                }

                if (p)
                {
                    pbuf* pout;
                    auto read_bytes = axl_ssl_read(self->ssl_obj, tpcb, p, &pout);
                    tcp_recved(tpcb, p->tot_len);
                    pbuf_free(p);

                    if (read_bytes <= SSL_OK)
                    {
                        handler(lwip::events::discon, *self, lwip::discon_reason::ssl_error);
                        return ERR_ABRT;
                    }

                    if (read_bytes > 0)
                    {
                        handler(lwip::events::recv, *self, lwip::buffer{pout});
                        return ERR_OK;
                    }
                }

                // conn closed
                handler(lwip::events::discon, *self, lwip::discon_reason::closed);
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
                handler(lwip::events::discon, *self, err == ERR_ABRT ?
                                        lwip::discon_reason::aborted : lwip::discon_reason::reset);
                system_os_post(tos::esp82::main_task_prio, 0, 0);
                axl_free(self->m_conn);
                self->m_conn = nullptr;
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

#ifdef TOS_HAVE_SSL
        struct secure_conn_state : conn_state
        {
            SSL* ssl_obj;
        };

        inline err_t handshake_receive(void *arg, struct tcp_pcb *tcp, struct pbuf *p, err_t err)
        {
            auto state = static_cast<secure_conn_state*>(arg);

            if(!tcp || !p) {
                state->res = ERR_CLSD;
                return ERR_ABRT;
            }

            system_soft_wdt_feed();

            struct pbuf* pout = nullptr;
            axl_ssl_read(state->ssl_obj, tcp, p, &pout);

            tcp_recved(tcp, p->tot_len);
            pbuf_free(p);
            if (pout)
            {
                pbuf_free(pout);
            }

            if (ssl_handshake_status(state->ssl_obj) == SSL_OK)
            {
                state->res = ERR_OK;
                state->sem.up();
                system_os_post(tos::esp82::main_task_prio, 0, 0);
            }

            return ERR_OK;
        }

        inline expected<secure_tcp_endpoint, connect_error> connect_ssl(wifi_connection&, ipv4_addr host, port_num_t port) {
            auto pcb = tcp_new();
            ip_addr_t a;
            memcpy(&a.addr, host.addr, 4);
            secure_conn_state state;

            tcp_arg(pcb, &state);
            tcp_recv(pcb, handshake_receive);
            tcp_err(pcb, conn_err_handler); // this signals ERR_CONN

            tcp_connect(pcb, &a, port.port, connected_handler); // this signals ERR_OK
            state.sem.down();

            if (state.res != ERR_OK)
            {
                return unexpected(connect_error::not_connected);
            }

            auto clientfd = axl_append(pcb);
            auto sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | SSL_SERVER_VERIFY_LATER, 1);

            system_update_cpu_freq(SYS_CPU_160MHZ); // Run faster during SSL handshake

            auto ext = ssl_ext_new();
            ssl_ext_set_max_fragment_size(ext, 2);
            auto sslObj = state.ssl_obj = ssl_client_new(sslContext, clientfd, nullptr, 0, ext);

            state.sem.down();

            if (state.res != ERR_OK)
            {
                return unexpected(connect_error::ssl_error);
            }

            system_update_cpu_freq(SYS_CPU_80MHZ); // back to normal speed

            if (state.res != ERR_OK)
            {
                return unexpected(connect_error::unknown);
            }

            return secure_tcp_endpoint(pcb, sslObj, sslContext);
        }
#endif
    }
}