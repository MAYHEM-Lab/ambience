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
void socket_runtime::run() {
    while (m_udp_sockets.empty() && m_tcp_listeners.empty() && m_tcp_sockets.empty()) {
        tos::println(log, "no sockets, waiting...");
        m_select_sem.down();
        if (!m_request_interruption) {
            tos::println(log, "[ERROR]", "wasn't woken up due to interruption");
        }
    }

    SlFdSet_t receive_set;
    SlFdSet_t write_set;
    SL_SOCKET_FD_ZERO(&receive_set);
    SL_SOCKET_FD_ZERO(&write_set);

    int16_t max = -1;

    for (auto& sock : m_udp_sockets) {
        SL_SOCKET_FD_SET(sock.native_handle(), &receive_set);
        max = std::max(max, sock.native_handle());
    }

    for (auto& sock : m_tcp_listeners) {
        SL_SOCKET_FD_SET(sock.native_handle(), &receive_set);
        max = std::max(max, sock.native_handle());
    }

    for (auto& sock : m_tcp_sockets) {
        SL_SOCKET_FD_SET(sock.native_handle(), &receive_set);
        //SL_SOCKET_FD_SET(sock.native_handle(), &write_set);
        max = std::max(max, sock.native_handle());
    }

    SlTimeval_t timeVal;
    timeVal.tv_sec = 0;
    timeVal.tv_usec = 0;

    tos::println(log, "calling select", max + 1);
    auto status = sl_Select(max + 1, &receive_set, &write_set, nullptr, &timeVal);
    while (status >= 0) {
        handle_select(receive_set, write_set);
        status = sl_Select(max + 1, &receive_set, &write_set, nullptr, &timeVal);
        tos::println(log, "select returned", int(status));
    }

    tos::println(log, "select returned", int(status));
    m_select_sem.down();

    if (m_request_interruption) {
        tos::println(log, "select got interrupted");
        m_request_interruption = false;
    }

    status = sl_Select(max + 1, &receive_set, &write_set, nullptr, &timeVal);
    tos::println(log, "after-select returned", int(status));
    if (status > 0) {
        handle_select(receive_set, write_set);
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
}

void socket_runtime::register_socket(udp_socket& socket) {
    m_udp_sockets.push_front(socket);
    m_request_interruption = true;
    m_select_sem.up();
}

void socket_runtime::register_socket(tcp_socket& socket) {
    m_tcp_sockets.push_front(socket);
    m_request_interruption = true;
    m_select_sem.up();
}

void socket_runtime::register_socket(tcp_listener& socket) {
    m_tcp_listeners.push_front(socket);
    m_request_interruption = true;
    m_select_sem.up();
}

void socket_runtime::remove_socket(udp_socket& socket) {
    m_udp_sockets.erase(std::find(m_udp_sockets.begin(), m_udp_sockets.end(), socket));
    m_request_interruption = true;
    m_select_sem.up();
}

void socket_runtime::remove_socket(tcp_socket& socket) {
    m_tcp_sockets.erase(std::find(m_tcp_sockets.begin(), m_tcp_sockets.end(), socket));
    m_request_interruption = true;
    m_select_sem.up();
}

void socket_runtime::remove_socket(tcp_listener& socket) {
    m_tcp_listeners.erase(std::find(m_tcp_listeners.begin(), m_tcp_listeners.end(), socket));
    m_request_interruption = true;
    m_select_sem.up();
}
} // namespace tos::cc32xx