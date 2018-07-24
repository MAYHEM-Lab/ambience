//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/span.hpp>
#include <drivers/common/inet/tcp_ip.hpp>
#include <stdint.h>
#include <tos/expected.hpp>

namespace tos
{
    namespace esp82
    {
        class wifi_connection
        {
        public:
            expected<ipv4_addr, bool> get_addr();
            wifi_connection(const wifi_connection&) = delete;
            wifi_connection(wifi_connection&& rhs) {
                rhs.discon = false;
            }
            ~wifi_connection();

        private:
            wifi_connection() = default;
            friend class wifi;
            bool discon = true;
        };

        enum class assoc_error
        {
            unknown
        };

        class wifi
        {
        public:
            wifi() noexcept;

            expected<wifi_connection, assoc_error>
            connect(span<const char> ssid, span<const char> passwd) noexcept;

            bool wait_for_dhcp();

            void scan();

            ~wifi();
        private:
        };

    }
}