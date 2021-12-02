//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#pragma once

#include "gpio.hpp"
#include "tos/fixed_fifo.hpp"
#include "tos/semaphore.hpp"
#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <nrfx_uarte.h>
#include <tos/mutex.hpp>
#include <tos/span.hpp>

namespace tos {
namespace nrf52 {
namespace detail {
inline const nrfx_uarte_t uarts[2]{{NRF_UARTE0, NRFX_UARTE0_INST_IDX},
                                   {NRF_UARTE1, NRFX_UARTE1_INST_IDX}};
} // namespace detail
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class uart
    : public self_pointing<uart>
    , public non_copy_movable {
public:
    explicit uart(const nrfx_uarte_t& dev,
                  usart_constraint&& config,
                  gpio::pin_type rx,
                  gpio::pin_type tx) noexcept;

    int write(span<const uint8_t> buf);
    span<uint8_t> read(span<uint8_t> buf);

    template<class AlarmT>
    tos::span<uint8_t>
    read(tos::span<uint8_t> b, AlarmT& alarm, std::chrono::milliseconds to) {
        lock_guard lk{m_read_busy};

        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        while (total < len) {
            auto res = m_read_sync.down(alarm, to);
            if (res == sem_ret::timeout) {
                break;
            }
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        return b.slice(0, total);
    }

    ~uart();

private:
    void default_read_cb(uint8_t data) {
        rx_buf.push_isr(data);
        m_read_sync.up_isr();
    }

    void handle_callback(const void* p_event);

    tos::mutex m_write_busy;
    tos::semaphore m_write_sync{0};

    tos::mutex m_read_busy;
    tos::semaphore m_read_sync{0};

    const nrfx_uarte_t* m_dev;

    tos::basic_fixed_fifo<uint8_t, 64, tos::ring_buf> rx_buf;
    uint8_t m_recv_byte;
    uint8_t m_recv_byte2;
};
} // namespace nrf52

inline nrf52::uart open_impl(devs::usart_t<0>,
                             nrf52::usart_constraint&& c,
                             nrf52::gpio::pin_type rx_pin,
                             nrf52::gpio::pin_type tx_pin) {
    return nrf52::uart{nrf52::detail::uarts[0], std::move(c), rx_pin, tx_pin};
}

inline nrf52::uart open_impl(devs::usart_t<1>,
                             nrf52::usart_constraint&& c,
                             nrf52::gpio::pin_type rx_pin,
                             nrf52::gpio::pin_type tx_pin) {
    return nrf52::uart{nrf52::detail::uarts[1], std::move(c), rx_pin, tx_pin};
}
} // namespace tos
