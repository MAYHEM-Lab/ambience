//
// Created by Mehmet Fatih BAKIR on 07/06/2018.
//

#include <usart.hpp>
#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>

namespace tos
{
    namespace arm
    {
        constexpr nrfx_uarte_t uart0 { NRF_UARTE0, NRFX_UARTE0_INST_IDX };

        uart::uart(usart_constraint&& config, gpio::pin_t rx, gpio::pin_t tx) noexcept
                : m_write_sync{0}, m_read_sync{0}
        {
            nrfx_uarte_config_t conf{};

            conf.interrupt_priority = 7;
            conf.parity = NRF_UARTE_PARITY_EXCLUDED;
            conf.hwfc = NRF_UARTE_HWFC_DISABLED;
            conf.baudrate = NRF_UARTE_BAUDRATE_115200;
            conf.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
            conf.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
            conf.pselrxd = rx;
            conf.pseltxd = tx;
            conf.p_context = this;

            auto res = nrfx_uarte_init(&uart0, &conf, [](nrfx_uarte_event_t const *p_event, void *p_context){
                auto self = static_cast<uart*>(p_context);
                self->handle_callback(p_event);
            });

            tos::refresh_interrupts();
        }

        void uart::handle_callback(const void *event) {
            auto p_event = static_cast<const nrfx_uarte_event_t*>(event);

            if (p_event->type == NRFX_UARTE_EVT_TX_DONE)
            {
                m_write_sync.up_isr();
            }
            else if (p_event->type == NRFX_UARTE_EVT_RX_DONE)
            {
                m_read_sync.up_isr();
            }
        }

        void uart::write(span<const char> buf) {
            tos::lock_guard<tos::mutex> lk { m_write_busy };

            nrfx_err_t err;
            if (nrfx_is_in_ram(buf.data()))
            {
                err = nrfx_uarte_tx(&uart0, (const uint8_t*)buf.data(), buf.size());
            }
            else
            {
                uint8_t tbuf[32];
                memcpy(tbuf, buf.data(), buf.size());
                err = nrfx_uarte_tx(&uart0, tbuf, buf.size());
            }

            m_write_sync.down();
        }

        void uart::read(span<char> buf) {
            tos::lock_guard<tos::mutex> lk { m_read_busy };

            auto err = nrfx_uarte_rx(&uart0, (uint8_t*)buf.data(), buf.size());

            m_read_sync.down();
        }
    }
}