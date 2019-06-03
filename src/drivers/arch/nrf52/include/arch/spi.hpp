//
// Created by fatih on 5/31/19.
//

#pragma once

#include <common/driver_base.hpp>
#include <nrfx_spim.h>
#include <arch/gpio.hpp>
#include <tos/span.hpp>

namespace tos
{
namespace nrf52
{

namespace details
{
    static inline const nrfx_spim_t spis[NRFX_SPIM_ENABLED_COUNT] {
        { NRF_SPIM0, NRFX_SPIM0_INST_IDX },
#if NRFX_CHECK(NRFX_SPIM1_ENABLED)
        { NRF_SPIM1, NRFX_SPIM1_INST_IDX },
#endif
#if NRFX_CHECK(NRFX_SPIM2_ENABLED)
        { NRF_SPIM2, NRFX_SPIM2_INST_IDX }
#endif
    };
}

class spi : public self_pointing<spi>, public tracked_driver<spi, NRFX_SPIM_ENABLED_COUNT>
{
public:
    using gpio_type = nrf52::gpio;

    explicit spi(gpio::pin_type sck, gpio::pin_type miso, gpio::pin_type mosi);

    uint8_t exchange(uint8_t byte) {
        exchange_many({&byte, 1});
        return byte;
    }

    void write(uint8_t byte)
    {
        write({&byte, 1});
    }

    void write(tos::span<const uint8_t> buffer) {
        nrfx_spim_xfer_desc_t desc;
        desc.p_tx_buffer = buffer.data();
        desc.tx_length = buffer.size();
        desc.p_rx_buffer = nullptr;
        desc.rx_length = 0;
        nrfx_spim_xfer(&details::spis[0], &desc, 0);
        sync.down();
    }

    void exchange_many(tos::span<uint8_t> buffer) {
        nrfx_spim_xfer_desc_t desc;
        desc.p_tx_buffer = desc.p_rx_buffer = buffer.data();
        desc.tx_length = desc.rx_length = buffer.size();
        nrfx_spim_xfer(&details::spis[0], &desc, 0);
        sync.down();
    }

    ~spi()
    {
        nrfx_spim_uninit(&details::spis[0]);
    }

private:

    void isr(nrfx_spim_evt_t const &)
    {
        sync.up_isr();
    }

    tos::semaphore sync{0};
};
}
}

// impl

namespace tos
{
namespace nrf52
{
    spi::spi(gpio::pin_type sck, gpio::pin_type miso, gpio::pin_type mosi) : tracked_driver(0) {
        nrfx_spim_config_t conf{};
        conf.sck_pin = detail::to_sdk_pin(sck);
        conf.miso_pin = detail::to_sdk_pin(miso);
        conf.mosi_pin = detail::to_sdk_pin(mosi);
        conf.irq_priority = 7;
        conf.ss_active_high = false;
        conf.mode = nrf_spim_mode_t::NRF_SPIM_MODE_0;
        conf.frequency = nrf_spim_frequency_t::NRF_SPIM_FREQ_1M;
        conf.bit_order = nrf_spim_bit_order_t::NRF_SPIM_BIT_ORDER_MSB_FIRST;
        conf.orc = 0xFF; // send 0xFF when TX finishes, but need to RX
        conf.ss_pin = NRFX_SPIM_PIN_NOT_USED;

        auto res = nrfx_spim_init(&details::spis[0], &conf, [](nrfx_spim_evt_t const * ev, void*) {
            spi::get(0)->isr(*ev);
        }, nullptr);

        if (res != NRFX_SUCCESS)
        {
            tos::kern::fatal("driver not initialized!");
        }
    }
} // namespace nrf52
} // namespace tos