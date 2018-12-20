//
// Created by fatih on 12/19/18.
//

#pragma once

#include <cstdint>
#include <tos/print.hpp>
#include <algorithm>

namespace tos
{
namespace ble
{
    struct address_t
    {
        uint8_t addr[6];
    };
} // namespace ble

template <class UsartT>
class hm10
{
public:
    explicit hm10(UsartT u) : m_usart{u} {}

    constexpr void reset();

    constexpr ble::address_t get_address() const;

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

    template<class UsartT>
    constexpr ble::address_t hm10<UsartT>::get_address() const {
        tos::print(m_usart, "AT+ADDR?");

        char buf[12]{};
        auto res = m_usart->read(buf);

        ble::address_t result{};
        std::copy(buf, buf + 12, result.addr);
        return result;
    }
} // namespace tos