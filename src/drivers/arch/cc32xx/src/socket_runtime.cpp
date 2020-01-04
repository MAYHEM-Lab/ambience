//
// Created by fatih on 12/26/19.
//

#include <arch/detail/sock_rt.hpp>
#include <arch/tcp.hpp>
#include <arch/udp.hpp>
#include <ti/drivers/net/wifi/simplelink.h>

extern "C" {
void SimpleLinkSocketTriggerEventHandler(SlSockTriggerEvent_t* pSlTriggerEvent) {
    tos::int_guard ig;
    switch (pSlTriggerEvent->Event) {
    case SL_SOCKET_TRIGGER_EVENT_SELECT:
        tos::cc32xx::socket_runtime::instance().select_event();
        break;
    }
}
}

namespace tos::cc32xx {
struct socket_runtime::select_sets {
    SlFdSet_t receive;
    SlFdSet_t write;
    int16_t max_fd = -1;

    select_sets() {
        SL_SOCKET_FD_ZERO(&receive);
        SL_SOCKET_FD_ZERO(&write);
    }
};

auto socket_runtime::make_select_set() const -> select_sets {
    select_sets res;

    for (auto& sock : m_udp_sockets) {
        SL_SOCKET_FD_SET(sock.native_handle(), &res.receive);
        res.max_fd = std::max(res.max_fd, sock.native_handle());
    }

    for (auto& sock : m_tcp_listeners) {
        SL_SOCKET_FD_SET(sock.native_handle(), &res.receive);
        res.max_fd = std::max(res.max_fd, sock.native_handle());
    }

    for (auto& sock : m_tcp_sockets) {
        SL_SOCKET_FD_SET(sock.native_handle(), &res.receive);
        // SL_SOCKET_FD_SET(sock.native_handle(), &res.write);
        res.max_fd = std::max(res.max_fd, sock.native_handle());
    }

    return res;
}

void socket_runtime::run() {
    while (m_udp_sockets.empty() && m_tcp_listeners.empty() && m_tcp_sockets.empty()) {
        tos::println(log, "no sockets, waiting...");
        m_count_sem.down();
        if (!m_request_interruption) {
            tos::println(log, "[ERROR]", "wasn't woken up due to interruption");
        }
    }

    SlTimeval_t timeVal{0, 0};

    auto sets = make_select_set();

    tos::println(log, "calling select", sets.max_fd + 1);
    auto status =
        sl_Select(sets.max_fd + 1, &sets.receive, &sets.write, nullptr, &timeVal);
    while (status >= 0) {
        handle_select(sets.receive, sets.write);
        status =
            sl_Select(sets.max_fd + 1, &sets.receive, &sets.write, nullptr, &timeVal);
        tos::println(log, "select returned", int(status));
    }

    tos::println(log, "select returned", int(status));
    m_select_sem.down();

    /*if (m_request_interruption) {
        tos::println(log, "select got interrupted");
        m_request_interruption = false;
        return run();
    }*/

    status = sl_Select(sets.max_fd + 1, &sets.receive, &sets.write, nullptr, &timeVal);
    tos::println(log, "after-select returned", int(status));
    if (status > 0) {
        handle_select(sets.receive, sets.write);
    }
}

void socket_runtime::handle_select(const SlFdSet_t& rx, const SlFdSet_t& write) {
    for (auto& sock : m_udp_sockets) {
        if (SL_SOCKET_FD_ISSET(sock.native_handle(), const_cast<SlFdSet_t*>(&rx))) {
            tos::println(log, "signalling udp receive on", int(sock.native_handle()));
            sock.signal_select();
        }
    }

    for (auto& sock : m_tcp_sockets) {
        if (SL_SOCKET_FD_ISSET(sock.native_handle(), const_cast<SlFdSet_t*>(&rx))) {
            tos::println(log, "signalling tcp receive on", int(sock.native_handle()));
            sock.signal_select_rx();
        }
        if (SL_SOCKET_FD_ISSET(sock.native_handle(), const_cast<SlFdSet_t*>(&write))) {
            tos::println(log, "signalling tcp transmit on", int(sock.native_handle()));
            sock.signal_select_tx();
        }
    }

    for (auto& sock : m_tcp_listeners) {
        if (SL_SOCKET_FD_ISSET(sock.native_handle(), const_cast<SlFdSet_t*>(&rx))) {
            tos::println(log, "signalling tcp accept on", int(sock.native_handle()));
            sock.signal_select();
        }
    }

    tos::this_thread::yield();
}

void socket_runtime::register_socket(udp_socket& socket) {
    m_udp_sockets.push_front(socket);
    //m_request_interruption = true;
    //m_select_sem.up();
    m_count_sem.up();
}

void socket_runtime::register_socket(tcp_socket& socket) {
    m_tcp_sockets.push_front(socket);
    //m_request_interruption = true;
    //m_select_sem.up();
    m_count_sem.up();
}

void socket_runtime::register_socket(tcp_listener& socket) {
    m_tcp_listeners.push_front(socket);
    //m_request_interruption = true;
    //m_select_sem.up();
    m_count_sem.up();
}

void socket_runtime::remove_socket(udp_socket& socket) {
    m_udp_sockets.erase(std::find(m_udp_sockets.begin(), m_udp_sockets.end(), socket));
    // m_request_interruption = true;
    // m_select_sem.up();
}

void socket_runtime::remove_socket(tcp_socket& socket) {
    m_tcp_sockets.erase(std::find(m_tcp_sockets.begin(), m_tcp_sockets.end(), socket));
    // m_request_interruption = true;
    // m_select_sem.up();
}

void socket_runtime::remove_socket(tcp_listener& socket) {
    m_tcp_listeners.erase(
        std::find(m_tcp_listeners.begin(), m_tcp_listeners.end(), socket));
    // m_request_interruption = true;
    // m_select_sem.up();
}
} // namespace tos::cc32xx