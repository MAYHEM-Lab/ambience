//
// Created by fatih on 12/26/19.
//

#pragma once

#include "detail/errors.hpp"
#include "detail/socket_base.hpp"

#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos::cc32xx {
class udp_socket : public socket_base<udp_socket> {
public:
    udp_socket();
    using socket_base::bind;
    using socket_base::native_handle;

    expected<void, network_errors> send_to(tos::span<const uint8_t> data,
                                           const tos::udp_endpoint_t& to);

    expected<span<uint8_t>, network_errors> receive_from(tos::span<uint8_t> data,
                                                tos::udp_endpoint_t& from);

    void signal_select() {
        m_receive_sem.up();
    }

private:
    semaphore m_receive_sem{0};
};
} // namespace tos::cc32xx
