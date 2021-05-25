#pragma once

#include "xbee/constants.hpp"
#include "xbee/request.hpp"
#include "xbee/response.hpp"
#include "xbee/types.hpp"
#include <stdint.h>
#include <tos/print.hpp>
#include <tos/span.hpp>
#include <tos/utility.hpp>

namespace tos {
template<class UsartT>
class xbee_s1 {
public:
    explicit xbee_s1(UsartT& dev) noexcept
        : m_usart{&dev} {
    }

    template<class TxPacket>
    void transmit(const TxPacket& packet);

private:
    UsartT* m_usart;
};
} // namespace tos


// IMPL

namespace tos {
template<class UsartT>
template<class TxPacket>
void xbee_s1<UsartT>::transmit(const TxPacket& packet) {
    xbee::write_to(*m_usart, packet);
}
} // namespace tos