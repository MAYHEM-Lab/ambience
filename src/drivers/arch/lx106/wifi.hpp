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
            wifi_connection(wifi_connection&& rhs) noexcept :
                m_state{rhs.m_state}
            {
                rhs.discon = false;
            }
            wifi_connection&operator=(wifi_connection&& rhs) noexcept
            {
                discon = rhs.discon;
                rhs.discon = false;
                return *this;
            }

            ~wifi_connection();

            bool ICACHE_FLASH_ATTR is_connected()  { return m_state != states::disconnected && m_state != states::null; }
            bool ICACHE_FLASH_ATTR is_disconnected() { return m_state == states::disconnected; }
            bool ICACHE_FLASH_ATTR has_ip() { return m_state == states::operational; }

            void ICACHE_FLASH_ATTR wait_for_dhcp();

            void consume_event() ICACHE_FLASH_ATTR;

        private:

            wifi_connection() = default;
            friend class wifi;
            bool discon = true;

            enum class states
            {
                operational,
                waiting_dhcp,
                disconnected,
                null
            } m_state = states::null;
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