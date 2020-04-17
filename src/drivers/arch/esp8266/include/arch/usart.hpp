//
// Created by fatih on 4/26/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <tos/driver_traits.hpp>
#include <tos/span.hpp>
#include <tos/thread.hpp>

namespace tos {
namespace esp82 {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

struct uart0 : self_pointing<uart0> {
public:
    uart0(usart_constraint);

    int write(span<const uint8_t>);
    span<uint8_t> read(span<uint8_t> buf);

    void isr();
};

struct sync_uart0 {
    int write(span<const uint8_t>);
};
} // namespace esp82

inline esp82::uart0 open_impl(devs::usart_t<0>, esp82::usart_constraint params) {
    return {std::move(params)};
}
} // namespace tos