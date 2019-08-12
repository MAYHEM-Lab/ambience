//
// Created by fatih on 6/28/18.
//

#pragma once

#include "../../../../../../../../../opt/x-tools/tos-esp-sdk/sdk/third_party/include/lwip/init.h"
#include "../../../../../../../../../opt/x-tools/tos-esp-sdk/sdk/third_party/include/lwip/tcp.h"
#include "../../../../../../libs/toscxx/include/algorithm"
#include "wifi.hpp"

#include <common/inet/lwip.hpp>
#include <common/inet/tcp_ip.hpp>
#include <tos/arch.hpp>
#include <tos/debug.hpp>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/intrusive_list.hpp>
#include <tos/track_ptr.hpp>

#ifdef TOS_HAVE_SSL
#include <lwipr_compat/lwipr_compat.h>
#undef putc
#undef getc
#undef printf
#include <ssl/tls1.h>
#endif

namespace tos {
namespace esp82 {
class tcp_endpoint : public non_copyable {
public:
    tcp_endpoint() = delete;

    tcp_endpoint(tcp_endpoint&& rhs) noexcept;
    tcp_endpoint& operator=(tcp_endpoint&&) = delete;

    template<class EventHandlerT>
    void attach(EventHandlerT& cb);

    uint16_t send(span<const char>);

    ~tcp_endpoint();

private:
    explicit tcp_endpoint(tcp_pcb* conn);
    friend class tcp_socket;

    template<class ConnHandlerT>
    friend err_t accept_handler(void* user, tcp_pcb* new_conn, err_t err);

    friend struct ep_handlers;

    friend expected<tcp_endpoint, lwip::connect_error>
    connect(wifi_connection&, ipv4_addr_t, port_num_t);

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
    explicit tcp_socket(tos::esp82::wifi_connection&, port_num_t port);
    ~tcp_socket();

    template<class ConnHandlerT>
    void accept(ConnHandlerT& handler);

    bool is_valid() const { return m_conn; }

private:
    tcp_pcb* m_conn = nullptr;
    void* m_accept_handler = nullptr;

    template<class ConnHandlerT>
    friend err_t accept_handler(void* user, tcp_pcb* new_conn, err_t err);
};

/**
 * Initiates a TCP connection with the given endpoint.
 *
 * @param iface the interface to connect through
 * @param host ip address of the destination host
 * @param port tcp port of the destination application
 * @return an endpoint or an error if connection fails
 */
expected<tcp_endpoint, lwip::connect_error>
connect(wifi_connection& iface, ipv4_addr_t host, port_num_t port);
} // namespace esp82
} // namespace tos

namespace tos {
namespace esp82 {
inline tcp_socket::tcp_socket(tos::esp82::wifi_connection&, port_num_t port) {
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
    /*auto res =*/handler(*self, tcp_endpoint{new_conn});
    system_os_post(tos::esp82::main_task_prio, 0, 0);
    return ERR_OK;
}

template<class ConnHandlerT>
void tcp_socket::accept(ConnHandlerT& handler) {

    m_accept_handler = &handler;

    tcp_arg(m_conn, this);
    tcp_accept(m_conn, &accept_handler<ConnHandlerT>);
}

inline tcp_socket::~tcp_socket() { tcp_close(m_conn); }

inline tcp_endpoint::tcp_endpoint(tcp_pcb* conn)
    : m_conn{conn} {
    tcp_arg(m_conn, this);
    tcp_err(m_conn, &null_err_handler);
}

inline tcp_endpoint::tcp_endpoint(tos::esp82::tcp_endpoint&& rhs) noexcept
    : m_conn{rhs.m_conn}
    , m_event_handler{rhs.m_event_handler} {
    tcp_arg(m_conn, this);
    rhs.m_event_handler = nullptr;
    rhs.m_conn = nullptr;
}

inline tcp_endpoint::~tcp_endpoint() {
#ifdef ESP_TCP_VERBOSE
    tos_debug_print("tcp dtor called\n");
#endif
    if (!m_conn)
        return;
#ifdef ESP_TCP_VERBOSE
    tos_debug_print("closing\n");
#endif
    tcp_recv(m_conn, nullptr);
    tcp_err(m_conn, nullptr);
    tcp_sent(m_conn, nullptr);
    tcp_close(m_conn);
    // this is required, as otherwise LWIP just keeps the control block
    // alive for as long as it can, don't delete it unless you solve the
    // deallocation problem!
    tcp_abort(m_conn);
}

inline uint16_t tcp_endpoint::send(tos::span<const char> buf) {
    if (!m_conn) {
        tos_debug_print("erroneous call to send");
        return 0;
    }
    tcp_write(m_conn, (uint8_t*)buf.data(), buf.size(), 0);
    tcp_output(m_conn);
    return buf.size();
}

struct ep_handlers {
    template<class CallbackT>
    static err_t
    recv_handler(void* user, struct tcp_pcb* tpcb, struct pbuf* p, err_t err) {
#ifdef ESP_TCP_VERBOSE
        tos_debug_print("recv stack: %p\n", read_sp());
#endif

        auto self = static_cast<tcp_endpoint*>(user);
        auto& handler = *(CallbackT*)self->m_event_handler;
        system_os_post(tos::esp82::main_task_prio, 0, 0);

        if (err != ERR_OK) {
#ifdef ESP_TCP_VERBOSE
            tos_debug_print("recv error\n");
#endif
            handler(lwip::events::discon, *self, lwip::discon_reason::recv_error);
            return ERR_OK;
        }
        if (p) {
            tcp_recved(tpcb, p->tot_len);
            handler(lwip::events::recv, *self, lwip::buffer{p});
        } else {
            // conn closed
#ifdef ESP_TCP_VERBOSE
            tos_debug_print("close received\n");
#endif
            handler(lwip::events::discon, *self, lwip::discon_reason::closed);
        }
        return ERR_OK;
    }

    template<class CallbackT>
    static err_t sent_handler(void* user, struct tcp_pcb*, u16_t len) {
#ifdef ESP_TCP_VERBOSE
        tos_debug_print("sent stack: %p\n", read_sp());
#endif

        auto self = static_cast<tcp_endpoint*>(user);

        auto& handler = *(CallbackT*)self->m_event_handler;
        handler(lwip::events::sent, *self, len);
        system_os_post(tos::esp82::main_task_prio, 0, 0);

        return ERR_OK;
    }

    template<class EventHandlerT>
    static void err_handler(void* user, err_t err) {
#ifdef ESP_TCP_VERBOSE
        tos_debug_print("err stack: %p\n", read_sp());
#endif
        auto self = static_cast<tcp_endpoint*>(user);

        auto& handler = *(EventHandlerT*)self->m_event_handler;
        handler(lwip::events::discon,
                *self,
                err == ERR_ABRT ? lwip::discon_reason::aborted
                                : lwip::discon_reason::reset);
        self->m_conn = nullptr;
        system_os_post(tos::esp82::main_task_prio, 0, 0);
#ifdef ESP_TCP_VERBOSE
        tos_debug_print("ok");
#endif
    }
};

inline void null_err_handler(void* user, err_t) {
    auto self = static_cast<tcp_endpoint*>(user);
    self->m_conn = nullptr;
    system_os_post(tos::esp82::main_task_prio, 0, 0);
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
    system_os_post(tos::esp82::main_task_prio, 0, 0);
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
    system_os_post(tos::esp82::main_task_prio, 0, 0);
}

inline expected<tcp_endpoint, lwip::connect_error>
connect(wifi_connection& c, ipv4_addr_t host, port_num_t port) {
    c.consume_all();
    if (!c.has_ip()) {
        return unexpected(lwip::connect_error::no_network);
    }
    auto pcb = tcp_new();
    ip_addr_t a;
    memcpy(&a.addr, host.addr, 4);
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
} // namespace esp82
} // namespace tos
