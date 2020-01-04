//
// Created by fatih on 11/6/19.
//

#pragma once

#include <ti/drivers/UART.h>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos::cc32xx {
class uart
    : public self_pointing<uart>
    , public tracked_driver<uart, 2>
    , public non_copy_movable {
public:
    explicit uart(int id);

    int write(tos::span<const uint8_t> data) {
        lock_guard lg{m_tx_guard};
        while (UART_write(native_handle(), data.data(), data.size()) != 0) {
            tos::this_thread::yield();
        }
        tos::kern::busy();
        m_tx_wait.down();
        tos::kern::unbusy();
        return data.size();
    }

    tos::span<uint8_t> read(span<uint8_t> data) {
        lock_guard lg{m_rx_guard};
        while (UART_read(native_handle(), data.data(), data.size()) != 0) {
            tos::this_thread::yield();
        }
        tos::kern::busy();
        m_rx_wait.down();
        tos::kern::unbusy();
        return data;
    }

    UART_Handle native_handle() {
        return m_handle;
    }

    ~uart() {
        UART_close(native_handle());
    }

    void rx_isr() {
        m_rx_wait.up_isr();
    }

    void tx_isr() {
        m_tx_wait.up_isr();
    }

private:
    mutex m_tx_guard;
    mutex m_rx_guard;
    semaphore m_tx_wait{0};
    semaphore m_rx_wait{0};
    UART_Handle m_handle;
};
} // namespace tos::cc32xx