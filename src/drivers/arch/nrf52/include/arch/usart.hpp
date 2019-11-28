//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <tos/mutex.hpp>
#include <tos/span.hpp>

namespace tos {
namespace nrf52 {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class uart
    : public self_pointing<uart>
    , public non_copy_movable {
public:
    explicit uart(usart_constraint&& config,
                  gpio::pin_type rx,
                  gpio::pin_type tx) noexcept;

    int write(span<const uint8_t> buf);
    span<uint8_t> read(span<uint8_t> buf);

    ~uart();

private:
    void handle_callback(const void* p_event);

    tos::mutex m_write_busy;
    tos::semaphore m_write_sync;

    tos::mutex m_read_busy;
    tos::semaphore m_read_sync;
};
} // namespace nrf52

inline nrf52::uart open_impl(devs::usart_t<0>,
                             nrf52::usart_constraint&& c,
                             nrf52::gpio::pin_type rx_pin,
                             nrf52::gpio::pin_type tx_pin) {
    return nrf52::uart{std::move(c), rx_pin, tx_pin};
}
} // namespace tos
