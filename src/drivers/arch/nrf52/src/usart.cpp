//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#include "tos/debug/log.hpp"
#include <arch/usart.hpp>

namespace tos {
namespace nrf52 {
static constexpr uint32_t compute_baudrate_register(uint32_t desired_rate) {
    // https://devzone.nordicsemi.com/f/nordic-q-a/391/uart-baudrate-register-values

    constexpr uint64_t multiplier = 4294967296UL;
    return uint64_t(desired_rate) * multiplier / 16'000'000UL;
}

static_assert(compute_baudrate_register(9600) == 0x275254);

uart::uart(const nrfx_uarte_t& dev,
           usart_constraint&& config,
           gpio::pin_type rx,
           gpio::pin_type tx) noexcept
    : m_dev{&dev} {
    nrfx_uarte_config_t conf{};

    conf.interrupt_priority = 7;
    conf.parity = NRF_UARTE_PARITY_EXCLUDED;
    conf.hwfc = NRF_UARTE_HWFC_DISABLED;
    conf.baudrate = nrf_uarte_baudrate_t(
        compute_baudrate_register(tos::get<tos::usart_baud_rate>(config).rate));
    conf.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
    conf.pselrxd = detail::to_sdk_pin(rx);
    conf.pseltxd = detail::to_sdk_pin(tx);
    conf.p_context = this;

    auto res = nrfx_uarte_init(
        m_dev, &conf, [](nrfx_uarte_event_t const* p_event, void* p_context) {
            auto self = static_cast<uart*>(p_context);
            self->handle_callback(p_event);
        });

    if (res != NRFX_SUCCESS) {
        LOG_ERROR("Could not init uart", res);
        // TODO: report error
    }

    tos::kern::refresh_interrupts();

    auto err = nrfx_uarte_rx(m_dev, &m_recv_byte, 1);
    if (err != NRFX_SUCCESS) {
        while (true)
            ;
    }
    err = nrfx_uarte_rx(m_dev, &m_recv_byte2, 1);
    if (err != NRFX_SUCCESS) {
        while (true)
            ;
    }
}

void uart::handle_callback(const void* event) {
    auto p_event = static_cast<const nrfx_uarte_event_t*>(event);

    if (p_event->type == NRFX_UARTE_EVT_TX_DONE) {
        m_write_sync.up_isr();
    } else if (p_event->type == NRFX_UARTE_EVT_RX_DONE) {
        auto& ev = p_event->data.rxtx;
        default_read_cb(*ev.p_data);
        nrfx_uarte_rx(m_dev, ev.p_data, ev.bytes);
    }
}

int uart::write(span<const uint8_t> buf) {
    lock_guard lk{m_write_busy};

    if (nrfx_is_in_ram(buf.data())) {
        auto err = nrfx_uarte_tx(m_dev, buf.data(), buf.size());
        if (err != NRFX_SUCCESS) {
            return -1;
            // TODO: report error
        }

        m_write_sync.down();
    } else {
        while (!buf.empty()) {
            uint8_t tbuf[32];
            auto len = std::min<size_t>(32, buf.size());
            std::copy_n(buf.begin(), len, std::begin(tbuf));
            buf = buf.slice(len);
            auto err = nrfx_uarte_tx(m_dev, tbuf, len);
            m_write_sync.down();
        }
    }
    return buf.size();
}

span<uint8_t> uart::read(span<uint8_t> b) {
    lock_guard lk{m_read_busy};
    size_t total = 0;
    auto len = b.size();
    auto buf = b.data();
    while (total < len) {
        m_read_sync.down();
        *buf = rx_buf.pop();
        ++buf;
        ++total;
    }
    return b.slice(0, total);
}

uart::~uart() {
    nrfx_uarte_uninit(m_dev);
}
} // namespace nrf52
} // namespace tos