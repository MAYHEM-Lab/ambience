//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/devices.hpp>
#include <tos/span.hpp>
#include <drivers/common/inet/tcp_ip.hpp>
#include <stdint.h>
#include <tos/expected.hpp>
extern "C"
{
#include <user_interface.h>
}

namespace tos
{
    namespace esp82
    {
        class wifi_connection
        {
        public:
            expected<ipv4_addr, bool> get_addr();
            wifi_connection(const wifi_connection&) = delete;
            wifi_connection(wifi_connection&& rhs) noexcept {
                rhs.discon = false;
            }
            wifi_connection&operator=(wifi_connection&& rhs) noexcept
            {
                discon = rhs.discon;
                rhs.discon = false;
                return *this;
            }
            ~wifi_connection();

            bool wait_for_dhcp() ICACHE_FLASH_ATTR;

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
            wifi() noexcept ICACHE_FLASH_ATTR;

            expected<wifi_connection, assoc_error>
            connect(span<const char> ssid, span<const char> passwd) noexcept ICACHE_FLASH_ATTR;

            void scan();

            ~wifi() ICACHE_FLASH_ATTR;
        private:
        };
    }

    namespace devs
    {
        template <int N> using wifi_t = dev<struct _wifi_t, N>;
        template <int N> static constexpr wifi_t<N> wifi{};
    }

    inline esp82::wifi open_impl(devs::wifi_t<0>)
    {
        return {};
    }
}