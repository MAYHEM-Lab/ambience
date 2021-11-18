//
// Created by fatih on 6/28/18.
//

#pragma once

#include <algorithm>
#include <common/inet/tcp_ip.hpp>
#include <lwip/tcp.h>
#include <tos/arch.hpp>
#include <tos/debug/debug.hpp>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/late_constructed.hpp>
#include <tos/lwip/lwip.hpp>
#include <tos/track_ptr.hpp>
#include <tos/task.hpp>

namespace tos::lwip {
class tcp_endpoint : public non_copyable {
public:
    tcp_endpoint() = delete;

    tcp_endpoint(tcp_endpoint&& rhs) noexcept;
    tcp_endpoint& operator=(tcp_endpoint&&) = delete;

    template<class EventHandlerT>
    void attach(EventHandlerT& cb);

    uint16_t send(span<const uint8_t>);
    tos::Task<uint16_t> async_send(span<const uint8_t>);

    ~tcp_endpoint();

private:
    explicit tcp_endpoint(tcp_pcb* conn);
    friend class tcp_socket;

    template<class ConnHandlerT>
    friend err_t accept_handler(void* user, tcp_pcb* new_conn, err_t err);

    friend struct ep_handlers;

    friend expected<tcp_endpoint, lwip::connect_error> connect(ipv4_addr_t, port_num_t);

    friend void null_err_handler(void* user, err_t err);

    tcp_pcb* m_conn;
    void* m_event_handler;
};

void null_err_handler(void* user, err_t err);

/**
 * This class implements a TCP listener
 */
class tcp_socket : public tos::list_node<tcp_socket> {
public:
    /**
     * Constructs a tcp socket with the given port on the given interface
     * @param port port to bind to
     */
    explicit tcp_socket(port_num_t port);
    ~tcp_socket();

    template<class ConnHandlerT>
    void async_accept(ConnHandlerT& handler);

    bool is_valid() const {
        return m_conn;
    }

private:
    tcp_pcb* m_conn = nullptr;
    void* m_accept_handler = nullptr;

    template<class ConnHandlerT>
    friend err_t accept_handler(void* user, tcp_pcb* new_conn, err_t err);
};

inline auto acceptor(tcp_socket& sock) {
    struct awaiter {
        bool await_ready() {
            return false;
        }

        void await_suspend(std::coroutine_handle<void()> res) {
            m_resume = res;
            m_sock->async_accept(*this);
        }

        bool operator()(tcp_socket& sock, tcp_endpoint&& ep) {
            m_ep.emplace(std::move(ep));
            return true;
        }

        tcp_endpoint await_resume() {
            return std::move(m_ep.get());
        }

        tcp_socket* m_sock;
        tos::late_constructed<tcp_endpoint> m_ep;
        std::coroutine_handle<void()> m_resume;
    };

    return awaiter{.m_sock = &sock};
}

/**
 * Initiates a TCP connection with the given endpoint.
 *
 * @param iface the interface to connect through
 * @param host ip address of the destination host
 * @param port tcp port of the destination application
 * @return an endpoint or an error if connection fails
 */
expected<tcp_endpoint, lwip::connect_error> connect(ipv4_addr_t host, port_num_t port);
} // namespace tos::lwip

namespace tos::lwip {
inline tcp_socket::tcp_socket(port_num_t port) {
    tos::lock_guard lg{tos::lwip::lwip_lock};
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

template<class ConnHandlerT>
err_t accept_handler(void* user, tcp_pcb* new_conn, err_t err) {
    if (err != ERR_OK) {
        return ERR_OK;
    }

    auto self = static_cast<tcp_socket*>(user);

    auto& handler = *(ConnHandlerT*)self->m_accept_handler;
    auto res = handler(*self, tcp_endpoint{new_conn});
    if (!res) {
        return ERR_ABRT;
    }
    return ERR_OK;
}

template<class ConnHandlerT>
void tcp_socket::async_accept(ConnHandlerT& handler) {
    m_accept_handler = &handler;
    tos::lock_guard lg{tos::lwip::lwip_lock};

    tcp_arg(m_conn, this);
    tcp_accept(m_conn, &accept_handler<ConnHandlerT>);
}

inline tcp_socket::~tcp_socket() {
    tcp_close(m_conn);
}

inline tcp_endpoint::tcp_endpoint(tcp_pcb* conn)
    : m_conn{conn} {
    tos::lock_guard lg{tos::lwip::lwip_lock};

    tcp_arg(m_conn, this);
    tcp_err(m_conn, &null_err_handler);
}

inline tcp_endpoint::tcp_endpoint(tcp_endpoint&& rhs) noexcept
    : m_conn{rhs.m_conn}
    , m_event_handler{rhs.m_event_handler} {
    tos::lock_guard lg{tos::lwip::lwip_lock};

    tcp_arg(m_conn, this);
    rhs.m_event_handler = nullptr;
    rhs.m_conn = nullptr;
}

inline tcp_endpoint::~tcp_endpoint() {
    //    LOG_TRACE("tcp dtor called");
    if (!m_conn) {
        return;
    }
    //    LOG_TRACE("closing");
    tos::lock_guard lg{tos::lwip::lwip_lock};

    tcp_recv(m_conn, nullptr);
    tcp_err(m_conn, nullptr);
    tcp_sent(m_conn, nullptr);
    tcp_close(m_conn);
    // this is required, as otherwise LWIP just keeps the control block
    // alive for as long as it can, don't delete it unless you solve the
    // deallocation problem!
    tcp_abort(m_conn);
}

inline uint16_t tcp_endpoint::send(tos::span<const uint8_t> buf) {
    if (!m_conn) {
        LOG_ERROR("erroneous call to send");
        return 0;
    }
    //    LOG("tcp send", buf.size());
    tos::lock_guard lg{tos::lwip::lwip_lock};

    auto write_res = tcp_write(m_conn, buf.data(), buf.size(), 0);
    //    LOG_TRACE("Write:", write_res);
    if (write_res != ERR_OK) {
        return 0;
    }
    //    auto out_res = tcp_output(m_conn);
    //    LOG_TRACE("Out:", out_res);
    return buf.size();
}

inline tos::Task<uint16_t> tcp_endpoint::async_send(tos::span<const uint8_t> buf) {
    if (!m_conn) {
        LOG_ERROR("erroneous call to send");
        co_return 0;
    }
    //    LOG("tcp send", buf.size());
    co_await tos::lwip::lwip_lock.async_lock();
    tos::unique_lock lg{tos::lwip::lwip_lock, tos::adopt_lock};

    auto write_res = tcp_write(m_conn, buf.data(), buf.size(), 0);
    //    LOG_TRACE("Write:", write_res);
    if (write_res != ERR_OK) {
        co_return 0;
    }
    //    auto out_res = tcp_output(m_conn);
    //    LOG_TRACE("Out:", out_res);
    co_return buf.size();
}
struct ep_handlers {
    template<class CallbackT>
    static err_t
    recv_handler(void* user, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
#ifdef TOS_TCP_VERBOSE
        tos_debug_print("recv stack: %p\n", tos_get_stack_ptr());
#endif

        auto self = static_cast<tcp_endpoint*>(user);
        auto& handler = *(CallbackT*)self->m_event_handler;

        if (err != ERR_OK) {
#ifdef TOS_TCP_VERBOSE
            tos_debug_print("recv error\n");
#endif
            handler(lwip::events::discon, *self, lwip::discon_reason::recv_error);
            return ERR_OK;
        }
        if (p) {
            handler(lwip::events::recv, *self, lwip::buffer{p});
            tcp_recved(tpcb, p->tot_len);
        } else {
            // conn closed
#ifdef TOS_TCP_VERBOSE
            tos_debug_print("close received\n");
#endif
            handler(lwip::events::discon, *self, lwip::discon_reason::closed);
        }
        return ERR_OK;
    }

    template<class CallbackT>
    static err_t sent_handler(void* user, struct tcp_pcb*, u16_t len) {
#ifdef TOS_TCP_VERBOSE
        tos_debug_print("sent stack: %p\n", tos_get_stack_ptr());
#endif

        auto self = static_cast<tcp_endpoint*>(user);

        auto& handler = *(CallbackT*)self->m_event_handler;
        handler(lwip::events::sent, *self, len);

        return ERR_OK;
    }

    template<class EventHandlerT>
    static void err_handler(void* user, err_t err) {
#ifdef TOS_TCP_VERBOSE
        tos_debug_print("err stack: %p\n", tos_get_stack_ptr());
#endif
        auto self = static_cast<tcp_endpoint*>(user);

        auto& handler = *(EventHandlerT*)self->m_event_handler;
        handler(lwip::events::discon,
                *self,
                err == ERR_ABRT ? lwip::discon_reason::aborted
                                : lwip::discon_reason::reset);
        self->m_conn = nullptr;
#ifdef TOS_TCP_VERBOSE
        tos_debug_print("ok");
#endif
    }
};

inline void null_err_handler(void* user, err_t) {
    auto self = static_cast<tcp_endpoint*>(user);
    self->m_conn = nullptr;
}

template<class EventHandlerT>
void tcp_endpoint::attach(EventHandlerT& cb) {
    m_event_handler = &cb;
    tcp_arg(m_conn, this);
    tcp_sent(m_conn, &ep_handlers::sent_handler<EventHandlerT>);
    tcp_recv(m_conn, &ep_handlers::recv_handler<EventHandlerT>);
    tcp_err(m_conn, &ep_handlers::err_handler<EventHandlerT>);
}

struct conn_state {
    tos::semaphore sem{0};
    err_t res;
};

inline auto connected_handler(void* arg, struct tcp_pcb*, err_t err) -> err_t {
    auto state = static_cast<conn_state*>(arg);
    state->res = err;
    state->sem.up();
    if (err != ERR_OK) {
        return ERR_ABRT;
    }
    return ERR_OK;
}

/**
 * Handles the err event of lwip during the initial TCP connection
 */
inline void conn_err_handler(void* arg, err_t err) {
    auto state = static_cast<conn_state*>(arg);
    state->res = err;
    state->sem.up();
}

inline expected<tcp_endpoint, lwip::connect_error> connect(ipv4_addr_t host,
                                                           port_num_t port) {
    auto pcb = tcp_new();
    ip_addr_t a;
    std::memcpy(&a.addr, host.addr.data(), 4);
    conn_state state;
    tcp_arg(pcb, &state);
    tcp_connect(pcb, &a, port.port, connected_handler); // this signals ERR_OK
    tcp_err(pcb, conn_err_handler);                     // this signals ERR_CONN
    state.sem.down();
    if (state.res == ERR_RST) {
        return unexpected(lwip::connect_error::connection_reset);
    }
    if (state.res != ERR_OK) {
        return unexpected(lwip::connect_error::unknown);
    }
    return tcp_endpoint(pcb);
}
} // namespace tos::lwip
