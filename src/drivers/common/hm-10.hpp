//
// Created by fatih on 12/19/18.
//

#pragma once

#include <cstdint>
#include <tos/print.hpp>
#include <algorithm>
#include <common/driver_base.hpp>
#include <tos/devices.hpp>

namespace tos
{
namespace ble
{
    struct address_t
    {
        uint8_t addr[6];
    };

    template <class StreamT>
    void print(StreamT& to, const address_t& addr)
    {
        for (auto p : addr.addr)
        {
            print(to, size_t(p), "");
        }
    }
} // namespace ble

/**
 * HM10s are TI CC2541 BLE modules that work through UART.
 *
 * The control mechanism works over AT commands. It's expected
 * that this driver handles all of the control messages, thus,
 * the user code should never transmit AT commands or receive
 * OK responses.
 *
 * @tparam UsartT type of the UART object this driver will use
 */
template <class UsartT>
class hm10 : public self_pointing<hm10<UsartT>>
{
public:
    explicit hm10(UsartT u) : m_usart{u} {}

    /**
     * Implements the AT+TEST functionality of the HM10 module
     *
     * Retries up to 3 times over 1.5 seconds for the module to
     * reply.
     *
     * @tparam AlarmT type of the alarm
     * @param alarm alarm object
     * @return whether the module has responded
     */
    template <class AlarmT>
    constexpr bool test(AlarmT& alarm)
    {
        int retry = 3;

        while (retry --> 0)
        {
            tos::print(m_usart, "AT");
            char buf[2]{};
            using namespace std::chrono_literals;
            auto r = m_usart->read(buf, alarm, 500ms);
            if (r.size() == 2 && r[0] == 'O' && r[1] == 'K') {
                return true;
            }
        }

        return false;
    }

    /**
     * Resets the HM10 module
     *
     * The driver shouldn't be used for 500ms after
     * calling this function.
     */
    constexpr void reset();

    /**
     * Reads the BLE MAC address from the HM10 module
     *
     * @return ble mac address
     */
    constexpr ble::address_t get_address() const;

    /**
     * Writes the given buffer to the connected device
     *
     * Use of AT commands are prohibited and may be
     * trapped by the driver.
     *
     * @param buf buffer to send
     * @return number of bytes sent
     */
    constexpr int write(span<const char> buf)
    {
        return m_usart->write(buf);
    }

    constexpr span<char> read(span<char> buf)
    {
        return m_usart->read(buf);
    }

    template <class AlarmT>
    constexpr span<char> read(span<char> buf, AlarmT& alarm, std::chrono::milliseconds to)
    {
        return m_usart->read(buf, alarm, to);
    }

    /**
     * Queries the module for the notification state
     *
     * HM10 devices support asynchronous messages to notify
     * the master controller when a connection is established and
     * when the connection drops.
     *
     * @return whether the notifications are enabled or not
     */
    constexpr bool notifications_enabled() const
    {
        tos::print(m_usart, "AT+NOTI?");
        char buf[8]{};
        auto res = m_usart->read(buf);
        return bool(res[7] - '0');
    }

    constexpr void enable_notifications()
    {

    }

private:

    mutable UsartT m_usart;
};
} // namespace tos

namespace tos
{
    template<class UsartT>
    constexpr void hm10<UsartT>::reset() {
        tos::print(m_usart, "AT+RESET");
    }

    inline uint8_t parse_hex(char c)
    {
        if (c >= '0' && c <= '9') { return c - '0'; }
        if (c >= 'A' && c <= 'F') { return 10 + c - 'A'; }
        if (c >= 'a' && c <= 'f') { return 10 + c - 'a'; }
        return -1;
    }

    inline uint8_t parse_hex(char high_nibble, char low_nibble)
    {
        return (parse_hex(high_nibble) << 4) | parse_hex(low_nibble);
    }

    template<class UsartT>
    constexpr ble::address_t hm10<UsartT>::get_address() const {
        tos::print(m_usart, "AT+ADDR?");

        char buf[20]{};
        auto res = m_usart->read(buf);

        ble::address_t result{};
        auto it = buf + 8;
        for (auto& a : result.addr)
        {
            auto high_nibble = *it++;
            a = parse_hex(high_nibble, *it++);
        }
        return result;
    }

    namespace devs
    {
        using hm10_t = dev<struct _hm10_t, 0>;
        static constexpr hm10_t hm10{};
    } // namespace devs

    template <class UartT>
    hm10<UartT> open_impl(devs::hm10_t, UartT&& uart)
    {
        return hm10<UartT>{ std::forward<UartT>(uart) };
    }
} // namespace tos
