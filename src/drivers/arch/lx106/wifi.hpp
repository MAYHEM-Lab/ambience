//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/span.hpp>
#include <drivers/common/inet/tcp_ip.hpp>
#include <stdint.h>

namespace tos
{
    namespace esp82
    {
        //TODO: return wifi_connection from wifi upon connection

        class wifi_connection
        {
        public:
            ipv4_addr get_addr();
        };

        class wifi
        {
        public:

            wifi() noexcept;

            bool connect(span<const char> ssid, span<const char> passwd) noexcept;

            bool wait_for_dhcp();

            void scan();

        private:
        };
    }
}