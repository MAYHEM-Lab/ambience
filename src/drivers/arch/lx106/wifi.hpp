//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/span.hpp>
#include <stdint.h>

namespace tos
{
    struct ipv4_addr
    {
        uint8_t addr[4];
    };
    namespace esp82
    {
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