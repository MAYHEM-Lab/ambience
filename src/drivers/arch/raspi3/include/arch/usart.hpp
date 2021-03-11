#pragma once

#include <arch/interrupts.hpp>
#include <common/usart.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ring_buf.hpp>
#include <tos/self_pointing.hpp>
#include <tos/semaphore.hpp>
#include <tos/soc/bcm283x.hpp>
#include <tos/span.hpp>
#include <tos/task.hpp>

namespace tos::raspi3 {
using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class uart0 : public self_pointing<uart0> {
public:
    uart0(usart_constraint&&, interrupt_controller& ic);

    int sync_write(tos::span<const uint8_t>);
    int write(tos::span<const uint8_t>);
    span<uint8_t> read(tos::span<uint8_t>);

    Task<int> async_write(tos::span<const uint8_t>) noexcept;

private:
    bool irq();
    irq_handler m_handler;

    semaphore m_sem{0};
    span<const uint8_t> m_sendbuf{nullptr};
    mutex m_lock;

    basic_fixed_fifo<uint8_t, 128, ring_buf> m_recv_buf;
    semaphore m_recv_sem{0};

    struct statistics {
        uint64_t irq_count;
        uint64_t overrun_count;
        uint64_t recv_irq_count;
        uint64_t send_irq_count;
    } m_stats{};
};
} // namespace tos::raspi3
namespace tos {
inline raspi3::uart0 open_impl(tos::devs::usart_t<0>,
                               raspi3::usart_constraint&& constraints,
                               raspi3::interrupt_controller& ic) {
    return raspi3::uart0{std::move(constraints), ic};
}
} // namespace tos
