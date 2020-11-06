#pragma once

#include <common/usart.hpp>
#include <tos/self_pointing.hpp>
#include <tos/soc/bcm283x.hpp>
#include <tos/span.hpp>

namespace tos::raspi3 {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class uart0 : public self_pointing<uart0> {
public:
    uart0(usart_constraint&&);

    int write(tos::span<const uint8_t>);
    span<uint8_t> read(tos::span<uint8_t>);

private:
};
} // namespace tos::raspi3
namespace tos {
inline raspi3::uart0 open_impl(tos::devs::usart_t<0>,
                               raspi3::usart_constraint&& constraints) {
    return raspi3::uart0{std::move(constraints)};
}
} // namespace tos
