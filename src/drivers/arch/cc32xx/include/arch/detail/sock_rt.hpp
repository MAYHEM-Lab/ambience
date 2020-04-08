//
// Created by fatih on 12/26/19.
//

#pragma once

#include "fwd.hpp"
#include "socket_base.hpp"

#include <common/usart.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

struct SlFdSet_t;

namespace tos::cc32xx {
class socket_runtime {
public:
    static socket_runtime& instance() {
        static socket_runtime socks;
        return socks;
    }

    void select_event() { m_select_sem.up_isr(); }

    void register_socket(tcp_listener& socket);
    void register_socket(tcp_socket& socket);
    void register_socket(udp_socket& socket);

    void remove_socket(tcp_listener& socket);
    void remove_socket(tcp_socket& socket);
    void remove_socket(udp_socket& socket);

    void run();

private:
    struct select_sets;
    void handle_select(const SlFdSet_t& rx, const SlFdSet_t& write);
    [[nodiscard]] select_sets make_select_set() const;

    bool m_request_interruption = false;
    semaphore m_select_sem{0};

    semaphore m_count_sem{0};

    intrusive_list<tcp_listener> m_tcp_listeners;
    intrusive_list<udp_socket> m_udp_sockets;
    intrusive_list<tcp_socket> m_tcp_sockets;
};
}