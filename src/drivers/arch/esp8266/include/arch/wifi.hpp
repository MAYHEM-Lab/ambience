//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/devices.hpp>
#include <tos/span.hpp>
#include <common/inet/tcp_ip.hpp>
#include "../../../../../../../../../opt/x-tools/tos-esp-sdk/xtensa-lx106-elf/lib/gcc/xtensa-lx106-elf/8.3.0/include/stdint.h"
#include <tos/expected.hpp>
#include <tos/track_ptr.hpp>
extern "C"
{
#include "../../../../../../../../../opt/x-tools/tos-esp-sdk/sdk/include/user_interface.h"
}

namespace tos
{
    namespace esp82
    {
        class wifi_connection : public tracked
        {
        public:
            expected<ipv4_addr_t, bool> get_addr();
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

            void consume_all() ICACHE_FLASH_ATTR;
            void consume_event() ICACHE_FLASH_ATTR;
            void consume_event(System_Event_t) ICACHE_FLASH_ATTR;

        private:
            wifi_connection();
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
            connect(span<const char> ssid) noexcept ICACHE_FLASH_ATTR { return connect(ssid, ""); }

            expected<wifi_connection, assoc_error>
            connect(span<const char> ssid, span<const char> passwd) noexcept ICACHE_FLASH_ATTR;

            mac_addr_t get_ether_address() const;

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