//
// Created by fatih on 6/27/18.
//

#pragma once

#include <tos/devices.hpp>
#include <tos/span.hpp>
#include <common/inet/tcp_ip.hpp>
#include <tos/expected.hpp>
#include <tos/track_ptr.hpp>
#include <cstdint>
extern "C"
{
#include "user_interface.h"
}

namespace tos
{
    namespace esp82
    {
        /**
         * This type represents an association or connection with a WiFi
         * access point.
         */
        class wifi_connection : public tracked
        {
        public:
            /**
             * Gets the current IP address of the connection.
             *
             * If the connection is invalid, returns an error
             *
             * @return current ip address
             */
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

            /**
             * Upon destruction, the connection is automatically
             * ended
             */
            ~wifi_connection();

            bool ICACHE_FLASH_ATTR is_connected()
            { return m_state != states::disconnected && m_state != states::null; }

            /**
             * Due to going out of range, AP resets etc, a wifi_connection object may
             * go to a disconnected state.
             *
             * Upon entering the disconnected state, the object becomes unusable and
             * must be destroyed. If re-establishing the connection is desired, it
             * must be re-initiated through the wifi driver.
             *
             * @return has the AP disconnected
             */
            bool ICACHE_FLASH_ATTR is_disconnected() { return m_state == states::disconnected; }
            bool ICACHE_FLASH_ATTR has_ip() { return m_state == states::operational; }

            /**
             * Blocks the calling thread until the DHCP process is complete
             */
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

        /**
         * This type implements the WiFi functionality on ESP8266 chips
         *
         * Currently, we only support Station mode, AP mode will be added
         * later
         */
        class wifi
        {
        public:
            wifi() noexcept ICACHE_FLASH_ATTR;

            expected<wifi_connection, assoc_error>
            connect(span<const char> ssid) noexcept ICACHE_FLASH_ATTR { return connect(ssid, ""); }

            /**
             * Connects to the access point with the given SSID and the password.
             *
             * If successful, a wifi_connection object will be returned that can be
             * used to open sockets etc.
             *
             * @param ssid SSID of the access point
             * @param passwd Password for the network
             * @return a wifi connection or error
             */
            expected<wifi_connection, assoc_error>
            connect(span<const char> ssid, span<const char> passwd) noexcept ICACHE_FLASH_ATTR;

            /**
             * Returns the MAC address of the WiFi module
             * @return mac address
             */
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