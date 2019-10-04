//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#include <arch/twim.hpp>
#include <tos/semaphore.hpp>

namespace tos
{
    namespace nrf52
    {
        static const nrfx_twim_t twim0 { NRF_TWIM0, NRFX_TWIM0_INST_IDX };

        twim::twim(gpio::pin_type clock_pin, gpio::pin_type data_pin)
        {
            nrfx_twim_config_t conf{};
            conf.frequency = NRF_TWIM_FREQ_100K;
            conf.interrupt_priority = 7;
            conf.hold_bus_uninit = false;
            conf.scl = detail::to_sdk_pin(clock_pin);
            conf.sda = detail::to_sdk_pin(data_pin);

            auto res = nrfx_twim_init(&twim0, &conf, [](nrfx_twim_evt_t const *p_event, void * instance){
                auto self = static_cast<twim*>(instance);
                self->m_event = p_event->type;
                self->m_event_sem.up_isr();
            }, this);

            if (res != NRFX_SUCCESS)
            {
                NVIC_SystemReset();
            }
        }

        twi_tx_res twim::transmit(twi_addr_t to, span<const char> buf) noexcept
        {
            nrfx_twim_enable(&twim0);
            tos::kern::refresh_interrupts();
            auto ret = nrfx_twim_tx(&twim0, to.addr, (const uint8_t*)buf.data(), buf.size(), false);

            if (ret != NRFX_SUCCESS)
            {
                return twi_tx_res::other;
            }

            m_event_sem.down();
            switch (m_event)
            {
            case NRFX_TWIM_EVT_ADDRESS_NACK: return twi_tx_res::addr_nack;
            case NRFX_TWIM_EVT_DATA_NACK: return twi_tx_res::data_nack;

            case NRFX_TWIM_EVT_DONE:
            default:
                return twi_tx_res::ok;
            }
        }

        twi_rx_res twim::receive(twi_addr_t from, span<char> buf) noexcept
        {
            nrfx_twim_enable(&twim0);
            tos::kern::refresh_interrupts();
            auto ret = nrfx_twim_rx(&twim0, from.addr, (uint8_t*)buf.data(), buf.size());

            if (ret != NRFX_SUCCESS)
            {
                return twi_rx_res::other;
            }

            m_event_sem.down();
            switch (m_event)
            {
            case NRFX_TWIM_EVT_ADDRESS_NACK: return twi_rx_res::addr_nack;
            case NRFX_TWIM_EVT_DATA_NACK: return twi_rx_res::data_nack;

            case NRFX_TWIM_EVT_DONE:
            default:
                return twi_rx_res::ok;
            }
        }
    }
}