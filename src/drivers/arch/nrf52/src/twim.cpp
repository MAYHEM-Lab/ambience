//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#include <arch/nrf52/twim.hpp>
#include <nrfx_twim.h>
#include <tos/semaphore.hpp>
#include <drivers/include/nrfx_twim.h>

namespace tos
{
    namespace nrf52
    {
        static constexpr nrfx_twim_t twim0 { NRF_TWIM0, NRFX_TWIM0_INST_IDX };

        struct data_t
        {
            nrfx_twim_evt_type_t evt;
            tos::semaphore sync{0};
        } data;

        twim::twim(gpio::pin_type clock_pin, gpio::pin_type data_pin)
        {
            nrfx_twim_config_t conf{};
            conf.frequency = NRF_TWIM_FREQ_250K;
            conf.interrupt_priority = 7;
            conf.hold_bus_uninit = false;
            conf.scl = clock_pin;
            conf.sda = data_pin;

            auto res = nrfx_twim_init(&twim0, &conf, [](nrfx_twim_evt_t const *p_event, void *p_context){
                data.evt = p_event->type;
                data.sync.up_isr();
            }, this);

            if (res != NRFX_SUCCESS)
            {
                NVIC_SystemReset();
            }
        }

        twi_tx_res twim::transmit(twi_addr_t to, span<const char> buf) noexcept
        {
            nrfx_twim_enable(&twim0);
            auto ret = nrfx_twim_tx(&twim0, to.addr, (const uint8_t*)buf.data(), buf.size(), false);

            if (ret != NRFX_SUCCESS)
            {
                return twi_tx_res::other;
                // reset?
            }

            data.sync.down();
            auto ev = data.evt;
            switch (ev)
            {
            case NRFX_TWIM_EVT_ADDRESS_NACK: return twi_tx_res::addr_nack;
            case NRFX_TWIM_EVT_DATA_NACK: return twi_tx_res::data_nack;

            case NRFX_TWIM_EVT_DONE:
            default:
                return twi_tx_res::ok;
            }
        }

        twi_tx_res twim::receive(twi_addr_t from, span<char> buf) noexcept
        {
            auto ret = nrfx_twim_rx(&twim0, from.addr, (uint8_t*)buf.data(), buf.size());

            if (ret != NRFX_SUCCESS)
            {
                // reset?
            }

            data.sync.down();
            auto ev = data.evt;
            switch (ev)
            {
            case NRFX_TWIM_EVT_ADDRESS_NACK: return twi_tx_res::addr_nack;
            case NRFX_TWIM_EVT_DATA_NACK: return twi_tx_res::data_nack;

            case NRFX_TWIM_EVT_DONE:
            default:
                return twi_tx_res::ok;
            }
        }
    }
}