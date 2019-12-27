//
// Created by fatih on 12/26/19.
//

#pragma once

#include "detail/errors.hpp"
#include "detail/events.hpp"

#include <common/inet/tcp_ip.hpp>
#include <tos/expected.hpp>

namespace tos::cc32xx {
class simplelink_wifi {
public:
    simplelink_wifi();
    ~simplelink_wifi();
private:
};
tos::mac_addr_t get_mac_address();
expected<void, network_errors> set_mac_address(tos::mac_addr_t address);
} // namespace tos::cc32xx