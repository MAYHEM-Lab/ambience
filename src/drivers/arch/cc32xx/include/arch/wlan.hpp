//
// Created by fatih on 12/26/19.
//

#pragma once

#include "detail/errors.hpp"
#include "detail/events.hpp"

#include <common/inet/tcp_ip.hpp>
#include <common/usart.hpp>
#include <tos/expected.hpp>

namespace tos::cc32xx {

struct iwifi_event_handler {
    virtual void handle(const ip_acquired&) = 0;
    virtual void handle(const wifi_connected&) = 0;
    virtual void handle(const wifi_disconnected&) = 0;
    virtual ~iwifi_event_handler() = default;
};

struct null_event_handler : iwifi_event_handler {
    void handle(const ip_acquired& acquired) override;
    void handle(const wifi_connected& connected) override;
    void handle(const wifi_disconnected& disconnected) override;
};

class simplelink_wifi {
public:
    simplelink_wifi();
    ~simplelink_wifi();

    void connect(std::string_view SSID, std::string_view password);

    void set_event_handler(iwifi_event_handler& handler) {
        m_ev_handler = &handler;
    }

private:
    null_event_handler def_handler;
    iwifi_event_handler* m_ev_handler = &def_handler;
    void thread();
};

tos::mac_addr_t get_mac_address();
expected<void, network_errors> set_mac_address(tos::mac_addr_t address);
} // namespace tos::cc32xx