//
// Created by fatih on 5/31/19.
//

#pragma once

#include <common/driver_base.hpp>
#include <nrfx_spim.h>
#include <arch/gpio.hpp>
#include <tos/span.hpp>
#include <optional>
#include <tos/expected.hpp>

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

enum class nrfx_errors {
    busy = NRFX_ERROR_BUSY,
    invalid_addr = NRFX_ERROR_INVALID_ADDR
};

class spi :
        public self_pointing<spi>,
        public tracked_driver<spi, NRFX_SPIM_ENABLED_COUNT>,
        public non_copy_movable
{
public:
    using gpio_type = nrf52::gpio;

    explicit spi(
            gpio::pin_type sck,
            std::optional<gpio::pin_type> miso,
            std::optional<gpio::pin_type> mosi);

    expected<void, nrfx_errors>
    write(tos::span<const uint8_t> buffer) {
        nrfx_spim_xfer_desc_t desc;
        desc.p_tx_buffer = buffer.data();
        desc.tx_length = buffer.size();
        desc.p_rx_buffer = nullptr;
        desc.rx_length = 0;
        auto res = nrfx_spim_xfer(&details::spis[0], &desc, 0);
        if (res != NRFX_SUCCESS)
        {
            return unexpected(static_cast<nrfx_errors>(res));
        }
        sync.down();

        return {};
    }

    expected<void, nrfx_errors>
    exchange(tos::span<uint8_t> buffer) {
        nrfx_spim_xfer_desc_t desc;
        desc.p_tx_buffer = desc.p_rx_buffer = buffer.data();
        desc.tx_length = desc.rx_length = buffer.size();
        auto res = nrfx_spim_xfer(&details::spis[0], &desc, 0);
        if (res != NRFX_SUCCESS)
        {
            return unexpected(static_cast<nrfx_errors>(res));
        }
        sync.down();
        return {};
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
    inline spi::spi(gpio::pin_type sck, std::optional<gpio::pin_type> miso, std::optional<gpio::pin_type> mosi)
        : tracked_driver(0) {
        nrfx_spim_config_t conf{};
        conf.sck_pin = detail::to_sdk_pin(sck);
        conf.miso_pin = miso ? detail::to_sdk_pin(*miso) : 0xFF;
        conf.mosi_pin = mosi ? detail::to_sdk_pin(*mosi) : 0xFF;
        conf.irq_priority = 7;
        conf.ss_active_high = false;
        conf.mode = nrf_spim_mode_t::NRF_SPIM_MODE_0;
        conf.frequency = nrf_spim_frequency_t::NRF_SPIM_FREQ_1M;
        conf.bit_order = nrf_spim_bit_order_t::NRF_SPIM_BIT_ORDER_MSB_FIRST;
        conf.orc = 0xFF; // send 0xFF when TX finishes, but need to RX
        conf.ss_pin = NRFX_SPIM_PIN_NOT_USED;

        auto res = nrfx_spim_init(&details::spis[0], &conf, [](nrfx_spim_evt_t const * ev, void*) {
            // forward the event handler to the spi instance
            // we only support 1 SPI atm, so the 0 is hard coded
            spi::get(0)->isr(*ev);
        }, nullptr);

        if (res != NRFX_SUCCESS)
        {
            tos::debug::panic("driver not initialized!");
        }
    }
} // namespace nrf52
} // namespace tos
